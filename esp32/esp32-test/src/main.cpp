#include "WiFiType.h"
#include "secret.h"
#include <Arduino.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <WiFi.h>
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

static void handleHealth() {
    String body;
    body.reserve(96);
    body += "uptime_ms = " + String(millis()) + " ms\n";
    body += "rssi = " + String(WiFi.RSSI()) + " db\n";
    body += "free_heap = " + String(ESP.getFreeHeap()) + " Bytes\n";
    server.send(200, "text/plain; charset=utf-8", body);
}

static void wifiBegin() {
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    lastWifiStatus = WiFi.status();
    Serial.printf("wifi: connecting to %s (start=%s)\n", WIFI_SSID, wifiStatusName(lastWifiStatus));
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
        case 'p':
            postEvent();
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

static void httpLoop() {
    if (WiFi.status() != WL_CONNECTED)
        return;

    if (!httpStarted) {
        server.on("/health", HTTP_GET, handleHealth);
        server.onNotFound([] { server.send(404, "text/plain; charset=utf-8", "not found\n"); });
        server.begin();
        httpStarted = true;
        Serial.println("http: listening on port 80");
    }

    server.handleClient();
}

void setup() {
    pinMode(kLedPin, OUTPUT);
    digitalWrite(kLedPin, LOW);

    Serial.begin(115200);
    delay(500);

    Serial.println();
    Serial.printf("=== Watt Esp Learning ===\n");
    Serial.printf("CPU: chip=%s cores=%u freq=%uMHz\n", ESP.getChipModel(), ESP.getChipCores(),
                  ESP.getCpuFreqMHz());
    Serial.printf("Flash size: %u bytes\n", ESP.getFlashChipSize());
    Serial.printf("Free heap: %u bytes\n\n", ESP.getFreeHeap());

    constexpr size_t kProbeBytes = 1024 * 1024;
    void* probe = malloc(kProbeBytes);
    if (probe == nullptr) {
        Serial.printf("malloc(%lu) failed as expected\n", static_cast<unsigned long>(kProbeBytes));
    } else {
        Serial.printf("malloc(%lu) unexpectedly succeeded\n",
                      static_cast<unsigned long>(kProbeBytes));
        free(probe);
    }

    wifiBegin();
}

void loop() {
    handleSerialInput();
    heartbeatLoop();
    wifiLoop();
    httpLoop();
}
