# Learning Record 0004: Media 模块 — 编解码器能力检测

## 日期

2026-06-16

## 学了什么

- Media 模块 19 个文件分三类：编解码器支持（4）、传输特性（2）、缓冲区配置（10）
- GstRegistryHasElementForCodec 的核心检测逻辑：找 decoder → 找 parser+decoder 链 → 返回 false
- 编译时缓存 GStreamer 工厂列表（static 变量），避免重复遍历 registry
- CodecToGstCaps 的映射表：Starboard 枚举 → GStreamer caps 字符串
- SbMediaIsVideoSupported 的判定顺序：decode-to-texture → AV1 环境变量 → 分辨率 → HDR → 白名单
- SbMediaIsAudioSupported 的 Dolby 支持检测（ENABLE_DOLBY 设备属性）
- SbMediaIsSupported 从 DRM 侧确认 codec 兼容性
- 缓冲区配置项的默认值（48MB video / 6MB audio / 128 对齐 / MEMORY_POOL）
- 上层检查 + PlayerCreate 再次检查的防御性设计

## 关键洞察

- Media 模块是"看门人"角色，所有检查通过后才创建 Player。但 PlayerCreate 内部仍然重复检查——上层检查用于 UI，下层是安全网
- Audio Opus 的 caps 生成最复杂：需要解析 audio_specific_config 生成精确的 GstCaps
- HDR 检查依赖于 DeviceInfoService（正向路径），体现了 Lesson 1 的双向适配器模式
- GStreamer 查询的"parser + decoder 回退"机制：没有 decoder 时可以通过 parser+decoder 链来支持

## 对后续学习的影响

Media 模块是相对浅层的模块。下一课可以进入 Audio Sink 模块或 System 模块
