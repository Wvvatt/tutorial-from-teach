#include <Arduino.h>
#include <esp_system.h>

namespace {
constexpr uint8_t kLedPin = 2;
constexpr uint32_t kDefaultIntervalMs = 1000;
uint32_t intervalMs = kDefaultIntervalMs;

uint32_t lastTickMs = 0;
uint32_t heartbeat = 0;
bool ledOn = false;
} // namespace

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
    Serial.printf("=== Watt Esp Learning ===\n");
    Serial.printf("CPU: chip=%s cores=%u freq=%uMHz\n", ESP.getChipModel(),
                  ESP.getChipCores(), ESP.getCpuFreqMHz());
    Serial.printf("Flash size: %u bytes\n", ESP.getFlashChipSize());
    Serial.printf("Free heap: %u bytes\n\n", ESP.getFreeHeap());

    constexpr size_t kProbeBytes = 1024 * 1024;
    void *probe = malloc(kProbeBytes);
    if (probe == nullptr) {
        Serial.printf("malloc(%lu) failed as expected\n",
                      static_cast<unsigned long>(kProbeBytes));
    } else {
        Serial.printf("malloc(%lu) unexpectedly succeeded\n",
                      static_cast<unsigned long>(kProbeBytes));
        free(probe);
    }
}

void loop() {
    handleSerialInput();
    const uint32_t now = millis();
    if (now - lastTickMs < intervalMs) {
        delay(1);
        return;
    }

    lastTickMs = now;
    ledOn = !ledOn;
    digitalWrite(kLedPin, ledOn ? HIGH : LOW);

    Serial.printf("heartbeat=%lu uptime_ms=%lu free_heap=%u gpio2=%s\n",
                  static_cast<unsigned long>(++heartbeat),
                  static_cast<unsigned long>(now), ESP.getFreeHeap(),
                  ledOn ? "HIGH" : "LOW");
}
