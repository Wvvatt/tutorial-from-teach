# 带读 vicodec 源码主路径

完成第六课后，用户开始把抽象的 V4L2 mem2mem 模型映射到一个实际内核驱动：`vicodec`。

## Status: active

## Evidence

- 理解 `vicodec_probe()` / `register_instance()` 如何注册 stateful encoder、stateful decoder 和 stateless decoder 三个 instance。
- 能把 `queue_init()` 与两条 vb2 queue 的 type、memory model、Request policy 对应起来。
- 理解 QBUF 进入 `buf_queue()` 后仍需经过 `job_ready()`，因此 QBUF 不等于 job 已执行。
- 能沿 `device_run()` 追踪 source/destination buffer、payload、metadata、`buffer_done` 和 `v4l2_m2m_job_finish()`。
- 重新定位 stateless Request 的源码生命周期：`v4l2_ctrl_request_setup()` → 本帧处理 → `v4l2_ctrl_request_complete()`。
- 能在 drain/DRC 代码中搜索 `V4L2_BUF_FLAG_LAST`、source-change 状态和 EOS event。

## Implications

- 下一节适合拿一段真实 decoder 驱动源码，按本课的五个入口做 guided code review。
- 如果暂时没有工作代码，可以继续练习 vicodec 的 `job_ready()` 和 `device_run()`，重点分析“为什么 job 被延迟”和“何时 buffer ownership 发生转移”。
- 用户已经从 API 使用层进入 driver callback / completion path 层，后续可以逐步加入锁、错误回收、DRC 和 drain 的细节。
