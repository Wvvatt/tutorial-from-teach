# 学完 Stateful Decoder 状态机

完成了第二课——Stateful Decoder 的完整状态机：UNINIT → INIT → CAPTURE_SETUP → DECODING → DRC/DRAIN/SEEK 各阶段的 ioctl 序列和转换规则。

## Status: active

## Evidence
- 完成了包含 4 道测验题的第二课
- 理解了 OUTPUT queue 是"master"、DRC 期间 OUTPUT 必须保持 streaming、drain 以 V4L2_BUF_FLAG_LAST 为标志等关键概念

## Implications
- 下一课可以进入 Stateless Decoder + Request API，形成完整的 decoder 知识体系
- 或者可以做一节实践课——用 vivid + v4l2-ctl 来验证学到的状态机概念
