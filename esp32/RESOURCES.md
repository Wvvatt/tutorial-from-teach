# ESP32 摄像头网络应用资源

## Knowledge

- [Arduino-ESP32 CameraWebServer example](https://github.com/espressif/arduino-esp32/tree/master/libraries/ESP32/examples/Camera/CameraWebServer)
  官方端到端示例，包含摄像头初始化、Wi‑Fi 连接、单张抓拍和流式 HTTP 服务。注意其中的 `board_config.h` 必须按实际板型选择引脚。
- [Espressif esp32-camera driver](https://github.com/espressif/esp32-camera)
  官方摄像头驱动；记录支持的传感器、`esp_camera_init`、`camera_fb_t` 和 JPEG 帧缓冲的用法。
- [Arduino-ESP32 documentation](https://docs.espressif.com/projects/arduino-esp32/en/latest/)
  Arduino 框架的官方 API 和安装、调试资料。
- [Arduino-ESP32 Serial API](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/serial.html)
  ESP32 `HardwareSerial`、`available()` 和 `read()` 的官方参考。用于串口输入和调试协议。
- [Arduino millis reference](https://docs.arduino.cc/language-reference/en/functions/time/millis/)
  `millis()` 的官方语义和返回值说明。用于所有非阻塞定时练习。
- [Arduino-ESP32 WiFi API](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/wifi.html)
  Wi‑Fi 状态、连接和断线处理的官方 API 参考。
- [ESP-IDF HTTP Server API](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/protocols/esp_http_server.html)
  ESP32 HTTP 服务端底层 API；当 Arduino `WebServer` 不够用时再查阅。
- [OV2640 datasheet](https://docs.espressif.com/projects/esp-dev-kits/en/latest/_static/esp32-s2-kaluga-1/datasheet/Camera_OV2640.pdf)
  传感器能力和输出格式的原始资料；用于理解分辨率、JPEG 和并行数据接口，不用于猜测开发板引脚。

## Wisdom (Communities)

- [Espressif Arduino GitHub Discussions](https://github.com/espressif/arduino-esp32/discussions)
  遇到特定板型、摄像头引脚或驱动版本问题时，优先搜索带有完整板型和日志的讨论。

## Gaps

- 当前还不知道开发板的准确型号和摄像头排线引脚映射。进入第 7 课前需要补齐这两个信息，否则不能安全复用 CameraWebServer 的板型配置。
