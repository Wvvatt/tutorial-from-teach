# Mission: Linux V4L2 视频解码子系统

## Why

工作中涉及 V4L2 decoder（解码器），需要深入理解 V4L2 的架构、buffer 管理机制、以及 stateful/stateless 两种解码器接口的工作原理，从而能在实际开发中正确使用和调试 V4L2 decoder 相关的代码。

## Success looks like

- 能解释 V4L2 的 two-queue 模型（OUTPUT / CAPTURE）并画出数据流
- 能区分 stateful decoder 与 stateless decoder 的核心差异和适用场景
- 能看懂并分析一个 V4L2 decoder 驱动的 init/decoding/drain/seek 流程
- 能用 v4l2-ctl 等工具调试 decoder 设备节点
- 能理解 Request API 在 stateless decoder 中的作用

## Constraints

- 在 macOS 上学习，无实际 V4L2 硬件可用
- 可通过 vivid 虚拟驱动或 QEMU 模拟环境进行实验
- 不定时学习，每节课应短小精悍

## Out of scope

- V4L2 capture (camera) 方向不深入涉及
- V4L2 encoder 暂不学习（除非与 decoder 对称的内容）
- Media Controller 框架的底层细节
- 具体视频编解码标准（H.264/H.265 等）的语法细节
