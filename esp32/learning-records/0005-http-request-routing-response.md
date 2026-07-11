# HTTP 请求、路由与响应闭环

用户能用自己的话解释 `server.on()` 负责注册路由、`server.send()` 负责返回响应、`server.handleClient()` 负责处理请求，并进一步理解 `handleClient()` 必须在 `loop()` 中反复获得控制权。当前真实工程已经加入 `/health`、404 路由和 WebServer 依赖。下一步可以转换观察视角：让 ESP32 从 HTTP 服务端变为客户端，主动向同网段 Mac 发送一条 JSON。

