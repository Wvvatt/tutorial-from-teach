# 教学备注

- 用户偏好中文、边做边学，要求每课都在真实 ESP32 上有可见结果。
- 用户已经完成：识别 `/dev/ttyUSB`、使用 PlatformIO 编译和烧录、使用串口监视器、理解 `partitions.bin`、认识 RTS/DTR。
- 当前硬件：经典 ESP32（芯片识别为 ESP32-D0WDQ6，4 MB Flash）和 OV2640 摄像头；Wi‑Fi 可与当前设备处于同一网段。
- 当前工程：`esp32-test`，Arduino + PlatformIO；串口稳定路径优先使用 `/dev/serial/by-id/usb-1a86_USB_Serial-if00-port0`。
- 课程偏好：先建立可观测的小闭环，再逐步引入网络、摄像头和服务端；不直接跳到大型 CameraWebServer 示例。
