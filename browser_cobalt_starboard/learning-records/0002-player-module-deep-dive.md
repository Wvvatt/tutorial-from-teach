# Learning Record 0002: Player 模块深度解析

## 日期

2026-06-15

## 学了什么

- Player 模块的三层架构：SbPlayer (opaque handle) → SbPlayerPrivate → PlayerImpl
- 播放器创建流程：参数校验 → 实例化 → GStreamer pipeline 构建 → 创建播放线程
- Pipeline 按 codec 类型和 DRM 状态动态拼接（h264parse vs h264secparse）
- WriteSample 数据流：样本入队 → DRM 解密 → appsrc push → GStreamer pipeline
- 状态机：kNull → kInitial → kInitialPreroll/kPrerollAfterSeek → kPresenting
- 双线程模型：Cobalt 主线程（API 调用）+ playback_thread（GMainLoop 事件循环）
- PendingSample 机制：密钥未就绪时暂存样本，就绪后按序列重放
- 缓冲管理：250ms 阈值暂停，2s 阈值恢复，每 80ms 轮询
- ForceStop: Application Freeze 时遍历所有播放器强制停止

## 关键洞察

- PendingSample 是处理 DRM 异步密钥的关键设计——样本和 key 可能乱序到达
- 内部状态机通过 GStreamer bus 消息（GST_MESSAGE_ASYNC_DONE）驱动状态转换
- pipeline_is_paused_internal_ 用于网络卡顿时暂停、缓冲恢复后自动播放
- 最多 2 个并发播放器的限制与 PIP 功能直接相关
- PlayerImpl 同时继承 Player（功能接口）和 DrmSystemWidevine::Observer（DRM 回调），是第一个课中提到的双向适配器模式的又一体现

## 对后续学习的影响

下一课可以深入 Widevine DRM 模块或 Media 编解码器能力检测模块
