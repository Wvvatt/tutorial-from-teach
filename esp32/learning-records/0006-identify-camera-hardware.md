# 用硬件证据确认摄像头板型

用户提供了开发板正反面和下载底板照片。可见证据包括主板 `ESP32-CAM` 丝印、OV2640 排线、经典 ESP32-S 模组、microSD 与闪光灯布局，以及独立下载底板 `HW-381 ESP32-CAM-MB V0.0.1`。该主板与 Espressif 官方 `CAMERA_MODEL_AI_THINKER` 引脚配置高度一致，因此后续可采用该 GPIO 表；但照片不足以证明制造商品牌，记录为“AI Thinker 引脚配置兼容型”而非“AI-Thinker 原厂”。初次使用通用 `esp32dev` 构建时 PSRAM 报告为 0；切换到 PlatformIO `esp32cam` 板型后，实机报告约 4 MB PSRAM 且 `psramFound()==true`，证明先前是构建配置未启用而非硬件缺失。
