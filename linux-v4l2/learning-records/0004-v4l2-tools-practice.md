# 学完 V4L2 调试工具实战

完成了第四课——v4l2-ctl、v4l2-compliance、vivid/vicodec/visl 虚拟驱动的使用方法和调试工作流。

## Status: active

## Evidence
- 完成了包含 5 道测验题的第四课
- 掌握了 v4l2-ctl 的核心命令：--list-devices、--all、--list-formats-out、--get-fmt-video-out、--stream-mmap、--decoder-cmd stop
- 理解了 v4l2-compliance 的用途和 -s / -m2m 选项
- 区分了 vivid（通用视频模拟）、vicodec（stateless decoder 模拟）、visl（多格式 stateless 测试框架）

## Implications
- 用户现在有能力在实际工作中用 v4l2-ctl 快速诊断 decoder 设备状态
- 可用 v4l2-compliance 验证驱动是否符合 V4L2 规范
- 下一步可深入驱动源码实现（hantro、cedrus 等真实 stateless 驱动），或学习 Media Controller 框架
- 或可在 Linux 环境中实际操作 vivid/vicodec 加深理解
