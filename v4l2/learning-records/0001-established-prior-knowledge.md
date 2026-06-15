# 已确认的先验知识

用户工作中涉及 V4L2 decoder，但对 stateful vs stateless 的区别尚不清楚。对 V4L2 整体架构的了解处于入门阶段。

## Status: active

## Evidence
- 用户选择了"还不确定"作为对 stateful/stateless decoder 区别的回答
- 学习动机是"工作中涉及到 v4l2 的 decoder"

## Implications
- 需从最基础的 V4L2 架构讲起（two-queue 模型、设备节点、buffer 管理）
- 后续课程可覆盖：stateful decoder 状态机（初始化解码→DRC→drain→seek）、stateless decoder + Request API、以及如何用 v4l2-ctl/v4l2-compliance 调试
- 在 macOS 上无法运行真实 V4L2 硬件，需考虑 vivid 虚拟驱动或 QEMU 方案
