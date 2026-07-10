# V4L2 学习资源

## Knowledge

- [Linux Kernel V4L2 API 文档 (kernel.org)](https://www.kernel.org/doc/html/latest/userspace-api/media/v4l/v4l2.html)
  V4L2 API 的完整规格说明。用於：查询具体 ioctl 的行为、buffer 类型定义、格式协商规则。

- [Stateful Video Decoder Interface](https://www.kernel.org/doc/html/latest/userspace-api/media/v4l/dev-decoder.html)
  内核文档中的 stateful decoder 接口规范。用於：理解 stateful decoder 的初始化/解码/drain/seek/DRC 流程。

- [Stateless Video Decoder Interface](https://www.kernel.org/doc/html/latest/userspace-api/media/v4l/dev-stateless-decoder.html)
  内核文档中的 stateless decoder 接口规范，包含 Request API 的使用。用於：理解 stateless decoder 与 stateful 的差异和 Request API 工作方式。

- [Linux Media Subsystem 文档首页](https://www.kernel.org/doc/html/latest/userspace-api/media/index.html)
  Linux Media 子系统总入口。用於：查找 V4L2、MC、MFC 等子模块文档。

- [v4l-utils 项目 (GitHub)](https://github.com/linuxtv/v4l-utils)
  V4L2 用户空间工具集（v4l2-ctl, v4l2-compliance 等）源码。用於：参考实现、调试工具用法。

- [V4L2 Memory-to-Memory Driver API](https://www.kernel.org/doc/html/latest/driver-api/media/v4l2-mem2mem.html)
  V4L2 mem2mem framework 的驱动侧 API。用於：理解 decoder 驱动如何调度 OUTPUT/CAPTURE buffer、`device_run`、`job_ready`、drain 相关 helper。

- [V4L2 videobuf2 Driver API](https://www.kernel.org/doc/html/latest/driver-api/media/v4l2-videobuf2.html)
  videobuf2 buffer 队列框架文档。用於：理解 `REQBUFS/QBUF/STREAMON/DQBUF` 如何映射到 `queue_setup`、`buf_prepare`、`buf_queue`、`start_streaming`、`stop_streaming`。

- [V4L2 API 详解 — linuxtv.org wiki](https://linuxtv.org/downloads/v4l-dvb-apis-new/userspace-api/v4l/v4l2.html)
  另一个入口访问 V4L2 规格。

## Wisdom (Communities)

- [linux-media 邮件列表](https://linuxtv.org/lists.php)
  V4L2 内核子系统的官方邮件列表。用於：提交 patch、询问驱动开发问题、追踪最新 API 变更。

- [LINUX TV 社区 (Discourse)](https://www.linux-media.org/)
  Linux media 子系统的社区论坛。

- Stack Overflow 的 [V4L2 标签](https://stackoverflow.com/questions/tagged/v4l2)
  用於：解决具体的用户空间编程问题。
