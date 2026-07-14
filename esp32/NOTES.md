# 教学备注

- 用户偏好中文、边做边学，要求每课都在真实 ESP32 上有可见结果。
- 用户已经完成：识别 `/dev/ttyUSB`、使用 PlatformIO 编译和烧录、使用串口监视器、理解 `partitions.bin`、认识 RTS/DTR。
- 当前硬件：经典 ESP32（芯片识别为 ESP32-D0WDQ6，4 MB Flash）和 OV2640 摄像头；照片显示主板为 AI Thinker 引脚配置兼容型 `ESP32-CAM`（高置信度，但无法仅凭照片确认原厂品牌），USB 下载底板丝印为 `HW-381 ESP32-CAM-MB V0.0.1`；Wi‑Fi 可与当前设备处于同一网段。
- 摄像头映射采用 Espressif 官方 `CAMERA_MODEL_AI_THINKER` 配置：PWDN=32, RESET=-1, XCLK=0, SIOD=26, SIOC=27, D0/Y2=5, D1/Y3=18, D2/Y4=19, D3/Y5=21, D4/Y6=36, D5/Y7=39, D6/Y8=34, D7/Y9=35, VSYNC=25, HREF=23, PCLK=22；闪光 LED=GPIO4。切换 PlatformIO `board = esp32cam` 后，实机确认约 4 MB PSRAM、`psramFound()==true`。
- 当前工程：`esp32-test`，Arduino + PlatformIO；Mac 串口当前为 `/dev/cu.usbserial-110`。
- 课程偏好：先建立可观测的小闭环，再逐步引入网络、摄像头和服务端；不直接跳到大型 CameraWebServer 示例。
