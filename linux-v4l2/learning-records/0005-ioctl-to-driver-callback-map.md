# 建立 ioctl 到驱动回调的源码阅读地图

完成第五课后，学习路线从用户态 V4L2 操作推进到驱动源码阅读：用户现在应把 `S_FMT/QBUF/STREAMON/DQBUF` 映射到 `v4l2_ioctl_ops`、`v4l2_m2m` helper、`vb2_ops` 和 `v4l2_m2m_ops.device_run`。这使后续课程可以直接带读真实 decoder 驱动（如 vicodec、hantro、cedrus），而不是继续停留在 API 流程层面。

## Status: active

## Implications

- 下一节适合选一个真实驱动，按 `v4l2_ioctl_ops` → `vb2_ops` → `v4l2_m2m_ops` → completion path 的顺序阅读。
- 可以开始讨论 driver-side buffer ownership、`vb2_buffer_done()`、drain/DRC 的 LAST buffer 实现细节。
