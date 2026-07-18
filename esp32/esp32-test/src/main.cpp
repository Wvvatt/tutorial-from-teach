#include "HardwareSerial.h"
#include "WiFiType.h"
#include "http_parser.h"
#include "secret.h"
#include <Arduino.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <WiFi.h>
#include <cstdlib>
#include <esp_camera.h>
#include <esp_system.h>

namespace {
constexpr uint8_t kLedPin = 2;
constexpr uint32_t kDefaultIntervalMs = 1000;
uint32_t intervalMs = kDefaultIntervalMs;

constexpr uint32_t kWifiCheckIntervalMs = 1000;
uint32_t lastWifiCheckMs = 0;
wl_status_t lastWifiStatus = WL_NO_SHIELD;
uint32_t wifiRetry = 0;
bool wifiIpPrinted = false;

uint32_t lastTickMs = 0;
uint32_t heartbeat = 0;
bool ledOn = false;

WebServer server(80);
bool httpStarted = false;

bool cameraReady = false;

constexpr const char kUploadUrl[] = "http://192.168.1.140:8000/images";
constexpr uint32_t kDefaultUploadIntervalMs = 10000;
constexpr uint32_t kMinUploadIntervalMs = 1000;
constexpr uint32_t kMaxUploadIntervalMs = 300000;
uint32_t uploadIntervalMs = kDefaultUploadIntervalMs;
uint32_t lastUploadAttemptMs = 0;
uint32_t uploadSequence = 0;
bool uploadEnabled = false;

constexpr uint32_t kMaxUploadBackoffMs = 60000;
constexpr int kCaptureFailed = -1000;

uint32_t currentUploadDelayMs = kDefaultUploadIntervalMs;
uint32_t uploadSuccessCount = 0;
uint32_t uploadFailureCount = 0;
uint8_t consecutiveUploadFailures = 0;
int lastUploadStatus = 0;
} // namespace

static const char* wifiStatusName(wl_status_t s) {
    switch (s) {
    case WL_NO_SHIELD:
        return "NO_SHIELD";
    case WL_IDLE_STATUS:
        return "IDLE";
    case WL_NO_SSID_AVAIL:
        return "NO_SSID";
    case WL_CONNECTED:
        return "CONNECTED";
    case WL_CONNECT_FAILED:
        return "CONNECT_FAILED";
    case WL_CONNECTION_LOST:
        return "CONNECTION_LOST";
    case WL_DISCONNECTED:
        return "DISCONNECTED";
    default:
        return "?";
    }
}

static const char* flashModeName(FlashMode_t mode) {
    switch (mode) {
    case FM_QIO:
        return "QIO";
    case FM_QOUT:
        return "QOUT";
    case FM_DIO:
        return "DIO";
    case FM_DOUT:
        return "DOUT";
    case FM_FAST_READ:
        return "FAST_READ";
    case FM_SLOW_READ:
        return "SLOW_READ";
    default:
        return "UNKNOWN";
    }
}

static void printHardwareInfo() {
    Serial.println("--- Hardware probe ---");

    Serial.println("[CPU]");
    Serial.printf("model=%s revision=%u architecture=Xtensa LX6\n", ESP.getChipModel(),
                  ESP.getChipRevision());
    Serial.printf("cores=%u frequency=%u MHz sdk=%s\n", ESP.getChipCores(), ESP.getCpuFreqMHz(),
                  ESP.getSdkVersion());

    Serial.println("[RAM: internal SRAM heap]");
    Serial.printf("total=%u bytes free=%u bytes\n", ESP.getHeapSize(), ESP.getFreeHeap());
    Serial.printf("minimum_free_since_boot=%u bytes largest_free_block=%u bytes\n",
                  ESP.getMinFreeHeap(), ESP.getMaxAllocHeap());

    Serial.println("[RAM: external PSRAM]");
    Serial.printf("found=%s total=%u bytes free=%u bytes\n", psramFound() ? "yes" : "no",
                  ESP.getPsramSize(), ESP.getFreePsram());
    Serial.printf("minimum_free_since_boot=%u bytes largest_free_block=%u bytes\n",
                  ESP.getMinFreePsram(), ESP.getMaxAllocPsram());

    Serial.println("[Flash]");
    Serial.printf("technology=external SPI NOR size=%u bytes\n", ESP.getFlashChipSize());
    Serial.printf("clock=%u MHz mode=%s firmware_size=%u bytes\n",
                  ESP.getFlashChipSpeed() / 1000000, flashModeName(ESP.getFlashChipMode()),
                  ESP.getSketchSize());
}

static void tryMalloc() {
    Serial.println("--- Probe Malloc ---");
    constexpr size_t kProbeBytes = 1024 * 1024;
    void* probe = malloc(kProbeBytes);
    if (probe == nullptr) {
        Serial.printf("malloc(%lu) failed\n", static_cast<unsigned long>(kProbeBytes));
    } else {
        Serial.printf("malloc(%lu) succeeded (%s)\n", static_cast<unsigned long>(kProbeBytes),
                      psramFound() ? "PSRAM enabled" : "internal heap only");
        free(probe);
    }
}

static void handleHealth() {
    String body;
    body.reserve(360);
    body += "uptime_ms = " + String(millis()) + " ms\n";
    body += "rssi = " + String(WiFi.RSSI()) + " db\n";
    body += "free_heap = " + String(ESP.getFreeHeap()) + " Bytes\n";

    const uint32_t elapsed = millis() - lastUploadAttemptMs;
    const uint32_t nextInMs =
        !uploadEnabled || elapsed >= currentUploadDelayMs ? 0 : currentUploadDelayMs - elapsed;

    body += "upload_enabled = " + String(uploadEnabled ? "yes" : "no") + "\n";
    body += "upload_interval_ms = " + String(uploadIntervalMs) + "\n";
    body += "upload_successes = " + String(uploadSuccessCount) + "\n";
    body += "upload_failures = " + String(uploadFailureCount) + "\n";
    body += "upload_consecutive_failures = " + String(consecutiveUploadFailures) + "\n";
    body += "upload_last_status = " + String(lastUploadStatus) + "\n";
    body += "upload_next_in_ms = " + String(nextInMs) + "\n";

    server.send(200, "text/plain; charset=utf-8", body);
}

static void handleGetConfig() {
    String body;
    body.reserve(32);
    body += "{\"interval_ms\":" + String(uploadIntervalMs) + "}\n";
    server.send(200, "application/json; charset=utf-8", body);
}

static bool parseUploadInterval(const String& raw, uint32_t& parsedIntervalMs) {
    if (raw.isEmpty()) {
        return false;
    }

    for (size_t index = 0; index < raw.length(); ++index) {
        if (raw[index] < '0' || raw[index] > '9') {
            return false;
        }
    }

    char* end = nullptr;
    const unsigned long parsed = std::strtoul(raw.c_str(), &end, 10);
    if (*end != '\0' || parsed < kMinUploadIntervalMs || parsed > kMaxUploadIntervalMs) {
        return false;
    }

    parsedIntervalMs = static_cast<uint32_t>(parsed);
    return true;
}

static void handlePostConfig() {
    if (!server.hasArg("interval_ms")) {
        server.send(400, "text/plain; charset=utf-8", "missing interval_ms\n");
        return;
    }

    uint32_t parsedIntervalMs = 0;
    if (!parseUploadInterval(server.arg("interval_ms"), parsedIntervalMs)) {
        server.send(400, "text/plain; charset=utf-8",
                    "interval_ms must be an integer from 1000 to 300000\n");
        return;
    }

    uploadIntervalMs = parsedIntervalMs;
    currentUploadDelayMs = uploadIntervalMs;
    lastUploadAttemptMs = millis();

    Serial.printf("config: upload interval=%lu ms\n",
                  static_cast<unsigned long>(uploadIntervalMs));
    handleGetConfig();
}

static void wifiBegin() {
    Serial.println("--- wifi begin ---");
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    lastWifiStatus = WiFi.status();
    Serial.printf("wifi: connecting to %s (start=%s)\n", WIFI_SSID, wifiStatusName(lastWifiStatus));
}

static bool cameraBegin() {
    Serial.println("--- camera begin ---");
    camera_config_t config{};
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;

    config.pin_d0 = 5;
    config.pin_d1 = 18;
    config.pin_d2 = 19;
    config.pin_d3 = 21;
    config.pin_d4 = 36;
    config.pin_d5 = 39;
    config.pin_d6 = 34;
    config.pin_d7 = 35;
    config.pin_xclk = 0;
    config.pin_pclk = 22;
    config.pin_vsync = 25;
    config.pin_href = 23;
    config.pin_sccb_sda = 26;
    config.pin_sccb_scl = 27;
    config.pin_pwdn = 32;
    config.pin_reset = -1;

    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    config.fb_location = CAMERA_FB_IN_PSRAM;

    const esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("camera: init failed err=0x%x\n", err);
        return false;
    }

    sensor_t* sensor = esp_camera_sensor_get();
    Serial.printf("camera: ready pid=0x%04x psram=%s\n", sensor->id.PID,
                  psramFound() ? "yes" : "no");
    return true;
}

static void captureOnce() {
    if (!cameraReady) {
        Serial.println("camera: not ready");
        return;
    }

    camera_fb_t* fb = esp_camera_fb_get();
    if (fb == nullptr) {
        Serial.println("camera: capture failed");
        return;
    }

    const bool jpegStart = fb->len >= 2 && fb->buf[0] == 0xFF && fb->buf[1] == 0xD8;
    const bool jpegEnd =
        fb->len >= 2 && fb->buf[fb->len - 2] == 0xFF && fb->buf[fb->len - 1] == 0xD9;

    Serial.printf("camera: %zux%zu bytes=%zu format=%d jpeg=%s\n", fb->width, fb->height, fb->len,
                  static_cast<int>(fb->format), jpegStart && jpegEnd ? "valid" : "invalid");

    esp_camera_fb_return(fb);
}

static void postEvent() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("post: skipped, wifi disconnected");
        return;
    }

    HTTPClient http;
    http.begin("http://192.168.1.140:8000/events");
    http.addHeader("Content-Type", "application/json");

    String body =
        "{\"uptime_ms\":" + String(millis()) + ",\"free_heap\":" + String(ESP.getFreeHeap()) + "}";
    const int status = http.POST(body);
    Serial.printf("post: status=%d body=%s\n", status, body.c_str());
    http.end();
}

static int uploadFrame() {
    camera_fb_t* fb = esp_camera_fb_get();
    if (fb == nullptr) {
        Serial.println("{\"event\":\"upload\",\"error\":\"capture_failed\"}");
        return kCaptureFailed;
    }

    const uint32_t sequence = ++uploadSequence;
    const size_t frameLength = fb->len;

    HTTPClient http;
    http.setConnectTimeout(2000);
    http.setTimeout(5000);
    http.begin(kUploadUrl);
    http.addHeader("Content-Type", "image/jpeg");
    http.addHeader("X-Image-Sequence", String(sequence));

    const int status = http.POST(fb->buf, frameLength);
    esp_camera_fb_return(fb);
    http.end();

    Serial.printf("{\"event\":\"upload\",\"seq\":%lu,\"bytes\":%zu,\"status\":%d}\n",
                  static_cast<unsigned long>(sequence), frameLength, status);
    return status;
}

static void heartbeatLoop() {
    const uint32_t now = millis();
    if (now - lastTickMs < intervalMs) {
        return;
    }

    lastTickMs = now;
    ledOn = !ledOn;
    digitalWrite(kLedPin, ledOn ? HIGH : LOW);

    Serial.printf("heartbeat=%lu uptime_ms=%lu free_heap=%u gpio2=%s\n",
                  static_cast<unsigned long>(++heartbeat), static_cast<unsigned long>(now),
                  ESP.getFreeHeap(), ledOn ? "HIGH" : "LOW");
}

static void wifiLoop() {
    const uint32_t now = millis();
    if (now - lastWifiCheckMs < kWifiCheckIntervalMs)
        return;
    lastWifiCheckMs = now;

    const wl_status_t s = WiFi.status();

    if (s == WL_CONNECTED) {
        if (!wifiIpPrinted) {
            Serial.printf("wifi: CONNECTED ip=%s rssi=%d retries=%lu\n",
                          WiFi.localIP().toString().c_str(), static_cast<int>(WiFi.RSSI()),
                          static_cast<unsigned long>(wifiRetry));
            wifiIpPrinted = true;
        }
        lastWifiStatus = s;
        return;
    }

    wifiIpPrinted = false; // 一旦掉线，下次连上要重新打印

    if (s != lastWifiStatus) {
        if (lastWifiStatus == WL_CONNECTED)
            ++wifiRetry;
        Serial.printf("wifi: %s -> %s retries=%lu\n", wifiStatusName(lastWifiStatus),
                      wifiStatusName(s), static_cast<unsigned long>(wifiRetry));
        lastWifiStatus = s;
    } else {
        Serial.print("wifi: retry...\n"); // 同一状态停留时，每 500ms 推进一下进度
    }
}

static void handleCapture() {
    if (!cameraReady) {
        server.send(503, "text/plain; charset=utf-8", "camera not ready\n");
        return;
    }

    camera_fb_t* fb = esp_camera_fb_get();
    if (fb == nullptr) {
        server.send(500, "text/plain; charset=utf-8", "capture failed\n");
        return;
    }

    server.sendHeader("Cache-Control", "no-store");
    server.setContentLength(fb->len);
    server.send(200, "image/jpeg", "");
    server.sendContent(reinterpret_cast<const char*>(fb->buf), fb->len);

    Serial.printf("http: GET /capture 200 bytes=%zu\n", fb->len);
    esp_camera_fb_return(fb);
}

static void handleStream() {
    if (!cameraReady) {
        server.send(503, "text/plain; charset=utf-8", "camera not ready\n");
        return;
    }

    WiFiClient client = server.client();
    client.print("HTTP/1.1 200 OK\r\n"
                 "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n"
                 "Cache-Control: no-store\r\n"
                 "Connection: close\r\n\r\n");

    uint32_t reportStartedMs = millis();
    uint32_t frames = 0;
    uint32_t bytesSent = 0;

    while (client.connected()) {
        camera_fb_t* fb = esp_camera_fb_get();
        if (fb == nullptr) {
            Serial.println("stream: capture failed");
            break;
        }

        const size_t frameLength = fb->len;
        client.printf("--frame\r\nContent-Type: image/jpeg\r\n"
                      "Content-Length: %zu\r\n\r\n",
                      frameLength);
        const size_t written = client.write(fb->buf, frameLength);
        client.print("\r\n");
        esp_camera_fb_return(fb);

        if (written != frameLength) {
            Serial.printf("stream: short write %zu/%zu\n", written, frameLength);
            break;
        }

        ++frames;
        bytesSent += static_cast<uint32_t>(written);
        const uint32_t now = millis();
        const uint32_t elapsedMs = now - reportStartedMs;
        if (elapsedMs >= 1000) {
            Serial.printf("stream: fps=%lu bytes_per_sec=%lu free_heap=%u\n",
                          static_cast<unsigned long>(frames * 1000 / elapsedMs),
                          static_cast<unsigned long>(bytesSent * 1000 / elapsedMs),
                          ESP.getFreeHeap());
            reportStartedMs = now;
            frames = 0;
            bytesSent = 0;
        }

        delay(1);
    }

    client.stop();
    Serial.println("stream: client disconnected");
}

static void httpLoop() {
    if (WiFi.status() != WL_CONNECTED)
        return;

    if (!httpStarted) {
        server.on("/health", HTTP_GET, handleHealth);
        server.on("/config", HTTP_GET, handleGetConfig);
        server.on("/config", HTTP_POST, handlePostConfig);
        server.on("/capture", HTTP_GET, handleCapture);
        server.on("/stream", HTTP_GET, handleStream);
        server.onNotFound([] { server.send(404, "text/plain; charset=utf-8", "not found\n"); });
        server.begin();
        httpStarted = true;
        Serial.println("http: listening on port 80");
    }

    server.handleClient();
}

static bool isPermanentUploadFailure(int status) {
    return status >= 400 && status < 500 && status != 408 && status != 429;
}

static void uploadLoop() {
    if (!uploadEnabled || WiFi.status() != WL_CONNECTED || !cameraReady) {
        return;
    }

    const uint32_t now = millis();
    if (now - lastUploadAttemptMs < currentUploadDelayMs) {
        return;
    }

    lastUploadAttemptMs = now;
    const int status = uploadFrame();
    lastUploadStatus = status;

    if (status == 201) {
        ++uploadSuccessCount;
        consecutiveUploadFailures = 0;
        currentUploadDelayMs = uploadIntervalMs;
        return;
    }

    ++uploadFailureCount;
    if (consecutiveUploadFailures < 255) {
        ++consecutiveUploadFailures;
    }

    if (isPermanentUploadFailure(status)) {
        uploadEnabled = false;
        currentUploadDelayMs = uploadIntervalMs;
        Serial.printf("{\"event\":\"upload_disabled\",\"status\":%d,"
                      "\"reason\":\"protocol_error\"}\n",
                      status);
        return;
    }

    if (currentUploadDelayMs < kMaxUploadBackoffMs) {
        currentUploadDelayMs *= 2;
        if (currentUploadDelayMs > kMaxUploadBackoffMs) {
            currentUploadDelayMs = kMaxUploadBackoffMs;
        }
    }

    Serial.printf("{\"event\":\"upload_backoff\",\"failures\":%u,"
                  "\"next_delay_ms\":%lu}\n",
                  static_cast<unsigned>(consecutiveUploadFailures),
                  static_cast<unsigned long>(currentUploadDelayMs));
}

static void handleSerialInput() {
    while (Serial.available() > 0) {
        const char command = static_cast<char>(Serial.read());

        switch (command) {
        case '1':
            intervalMs = 250;
            break;
        case '2':
            intervalMs = 1000;
            break;
        case '3':
            intervalMs = 2500;
            break;
        case 'r':
            lastWifiStatus = WL_NO_SHIELD;
            wifiIpPrinted = false;
            WiFi.reconnect();
            return;
        case 'p':
            postEvent();
            return;
        case 'c':
            captureOnce();
            return;
        case 'u':
            uploadEnabled = !uploadEnabled;
            consecutiveUploadFailures = 0;
            currentUploadDelayMs = uploadIntervalMs;
            lastUploadAttemptMs = millis();
            Serial.printf("upload: %s interval=%lu ms\n", uploadEnabled ? "enabled" : "disabled",
                          static_cast<unsigned long>(currentUploadDelayMs));
            return;
        case '\r':
        case '\n':
            return;
        default:
            Serial.printf("unknown command: %c\n", command);
            return;
        }

        Serial.printf("interval set to %u\n", intervalMs);
    }
}

void setup() {
    pinMode(kLedPin, OUTPUT);
    digitalWrite(kLedPin, LOW);

    Serial.begin(115200);
    delay(500);

    Serial.println();
    Serial.println("=== Watt Esp Setup Start ===");
    printHardwareInfo();
    tryMalloc();

    wifiBegin();
    cameraReady = cameraBegin();
    Serial.println("=== Watt Esp Setup End ===");
    Serial.println("");
}

void loop() {
    handleSerialInput();
    heartbeatLoop();
    wifiLoop();
    httpLoop();
    uploadLoop();
}
