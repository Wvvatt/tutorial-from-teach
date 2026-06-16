# Curriculum

7 课，自上而下按数据流方向编排。

```
JS API → Blink → Demuxer → Decoder → Compositor → Platform → DRM
 001      002      003        004        005          006       007
```

| # | 名称 | 要点 |
|---|------|------|
| 001 | Browser Video API | `<video>` / MSE / EME / 本地 MP4 实际播放 + 事件流模拟器 |
| 002 | Blink 层 | HTMLMediaElement → WebMediaPlayerImpl / Mojo 分界 / bridge 模拟器 |
| 003 | Demuxer 层 | FFmpegDemuxer / ChunkDemuxer / SourceBufferStream / appendBuffer 模拟器 |
| 004 | Decoder 抽象 | VDA / media::VideoDecoder / MojoVideoDecoder / codec 检测器 |
| 005 | Compositor + VideoNG | overlay vs compositor 路径 / VideoFrameCompositor / overlay 开关演示 |
| 006 | Linux 平台层 | dma-buf / Wayland / KMS / hardware overlay planes / GStreamer 集成 / plane budget 模拟器 |
| 007 | EME + DRM + TEE | EME / Widevine L1 vs L3 / TEE / secure path / Clear DASH + Widevine CENC 实际播放 / EME 状态机模拟器 |

## 页面结构

- `lessons/*.html` 使用统一的 `assets/course.css` 和 `assets/course.js`。
- Lesson 001 包含本地 MP4 实际播放，不依赖网络。
- Lesson 007 包含真实 DASH/DRM 播放；dash.js 固定为本地 vendor 文件，MPD/license 使用 Widevine 官方测试源。
