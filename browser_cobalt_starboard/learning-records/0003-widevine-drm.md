# Learning Record 0003: Widevine DRM 内容保护

## 日期

2026-06-15

## 学了什么

- Widevine DRM 在 AML 上的三层架构：SbDrmSystem → DrmSystemWidevine → wv3cdm
- License 获取的完整生命周期：createSession → generateRequest → onMessage → update → onKeyStatusesChange
- 初始化过程：EnsureWidevineCdmIsInitialized 只执行一次，传入 ClientInfo + Storage + Clock + Timer
- 两条解密路径：Starboard 标准 InputBuffer 路径（空实现）vs GStreamer GstBuffer 路径（实际解密）
- DecryViaWv 是真正的硬件解密入口，通过 secmem handle 传递到 TrustZone
- 密钥状态管理：session_keys_ map + OnAllKeysUpdated → SbEventSchedule → AnnounceKeys 链
- Observer 模式：DrmSystemWidevine 作为 Subject，PlayerImpl 作为 Observer

## 关键洞察

- SbDrmSystemPrivate::Decrypt(InputBuffer*) 空实现说明 AML 的解密绕过了 Starboard 标准路径——所有解密在 PlayerImpl 的 WriteSample 中通过 GStreamer 路径完成
- AnnounceKeys 通过 SbEventSchedule 延迟执行，而非直接通知，这是为了避免在 CDM 回调中修改观察者列表造成的竞态
- cached_ready_keys_ 缓存所有就绪的密钥 ID，Observers 在 AnnounceKeys 时遍历这些 key 逐个通知
- DRM 模块也采用 Observer 模式（与第一课的 PlatformEvent 类似），但使用显式 AddObserver/RemoveObserver 而非事件总线

## 对后续学习的影响

下一课可以进入 Media 模块（编解码器能力检测）或 system 模块（系统属性、扩展接口）
