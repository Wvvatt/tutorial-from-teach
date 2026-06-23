# 学完 Stateless Decoder 与 Request API

完成了第三课——Stateless Decoder 的每帧独立提交模型、Request API 的原子绑定机制、以及 stateful vs stateless 的完整对比。

## Status: active

## Evidence
- 完成了包含 5 道测验题的第三课
- 理解了 stateless decoder 的核心差异：驱动为零状态，用户维护全部解码上下文
- 掌握了 Request API 的生命周期：ALLOC → PREPARE(QBUF+controls) → QUEUE → COMPLETE(poll) → REINIT/close
- 理解了 HOLD_CAPTURE_BUF 在多 slice 帧处理中的作用
- 能区分 stateful 和 stateless 在 DRC、drain、seek 上的流程差异

## Implications
- 用户已有完整的 decoder 理论知识（V4L2 基础 → stateful 状态机 → stateless + Request API）
- 下一课应做实践课：vivid 虚拟驱动 + v4l2-ctl 实际操作
- 或深入 codec 层面：具体看一个 stateless driver 的实现（如 hantro、cedrus）
