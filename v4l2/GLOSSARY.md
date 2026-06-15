# V4L2 Glossary

V4L2 术语表。本 workspace 中的所有课程和文档遵循如下术语定义。

## 核心架构

**V4L2 (Video for Linux 2)**:
Linux 内核中视频设备的用户空间 API 与驱动框架。涵盖视频采集、输出、编解码、VBI、Radio 等。

**V4L2 device node**:
V4L2 驱动的用户空间接口，表现为 `/dev/videoX` 等字符设备。

**OUTPUT queue**:
V4L2 mem2mem 设备中用户填充输入数据的 buffer 队列。对 decoder 而言，是编码码流（bytestream）的输入队列。类型为 `V4L2_BUF_TYPE_VIDEO_OUTPUT`（或 _MPLANE）。
_Avoid_: 输入队列、source queue

**CAPTURE queue**:
V4L2 mem2mem 设备中输出解码/编码结果的 buffer 队列。对 decoder 而言，是解码后原始帧的输出队列。类型为 `V4L2_BUF_TYPE_VIDEO_CAPTURE`（或 _MPLANE）。
_Avoid_: 输出队列、destination queue

**memory-to-memory (mem2mem)**:
一类 V4L2 设备，从 OUTPUT queue 读取数据，处理后写入 CAPTURE queue。decoder 和 encoder 都是典型的 mem2mem 设备。

## Buffer & 内存

**MMAP**:
V4L2 通过 `mmap()` 在用户空间和内核空间共享 buffer 的模式。由驱动分配物理内存，用户态通过 mmap 映射访问。
_Avoid_: memory-mapped buffers

**USERPTR**:
用户空间自行分配内存，通过指针传递给驱动使用的 buffer 模式。

**DMABUF**:
通过 dma-buf 文件描述符在不同设备间共享 buffer 的模式。常用于 decoder → display / GPU 的零拷贝路径。
_Avoid_: dma-buf importing

**REQUEST**:
用于 stateless decoder 的 buffer 模式，将多个操作（queue buffer + set controls + submit）绑定到一个 Request 中原子提交。

**v4l2_buffer**:
V4L2 buffer 的数据结构，描述 buffer 的索引、类型、状态、时间戳、数据长度等信息。

**v4l2_plane**:
multi-planar 格式中描述单个平面的数据结构（如 Y 平面、UV 平面各自独立描述）。
_Avoid_: plane descriptor

## Decoder 专用术语

**coded format**:
编码/压缩的视频码流格式（如 H.264, VP8, HEVC/H.265）。对应 decoder OUTPUT queue 的像素格式。
_Avoid_: compressed format, encoded format

**raw format**:
未压缩的原始像素格式（如 NV12, YUYV, RGB32）。对应 decoder CAPTURE queue 的像素格式。

**coded resolution**:
码流中编码分辨率，按 codec 宏块对齐后的像素宽高。可能大于 visible resolution。
_Avoid_: stream resolution, aligned resolution

**visible resolution**:
显示分辨率，码流中实际可视画面的像素宽高，≤ coded resolution。

**keyframe**:
不依赖之前帧即可独立解码的帧（如 H.264 IDR 帧、VP8/9 关键帧）。
_Avoid_: I-frame

**resume point**:
码流中解码可以从之开始/继续的位置。例如：关键帧、或 SPS/PPS/IDR 序列。

**DPB (Decoded Picture Buffer)**:
H.264/HEVC 中存储已解码参考帧的缓冲池。

**decode order**:
帧被解码器处理的顺序。可能与 display order 不同（当存在帧重排时）。

**display order**:
帧被显示的顺序。

**SPS (Sequence Parameter Set)**:
H.264/HEVC 码流中包含编码参数（分辨率、profile/level 等）的元数据单元。

**PPS (Picture Parameter Set)**:
H.264/HEVC 码流中包含每帧编码参数（熵编码模式、slice 分组等）的元数据单元。

## Decoder 类型

**Stateful decoder**:
由驱动/硬件维护解码状态（DPB、参考帧列表等），用户只需送入完整码流，按 display order 取出解码帧。

**Stateless decoder**:
驱动不在帧之间保留解码状态，由用户空间维护所有解码上下文，每帧解码需通过 Request API 提供完整的解码参数。
_Avoid_: stateless codec, pure hardware decoder

## 关键 ioctl

**VIDIOC_REQBUFS**:
请求分配指定数量的 V4L2 buffer。初始化 queue 的第一步。
_Avoid_: reqbufs, request buffers

**VIDIOC_QBUF**:
将一个 buffer 放入队列（交给驱动处理）。
_Avoid_: qbuf, queue buffer

**VIDIOC_DQBUF**:
从队列中取出一个已处理的 buffer（从驱动取回）。
_Avoid_: dqbuf, dequeue buffer

**VIDIOC_STREAMON**:
启动指定 queue 的 streaming。
_Avoid_: stream on

**VIDIOC_STREAMOFF**:
停止指定 queue 的 streaming。
_Avoid_: stream off

**VIDIOC_S_FMT / VIDIOC_G_FMT**:
设置/获取 queue 的格式（分辨率、像素格式等）。

**V4L2_EVENT_SOURCE_CHANGE**:
由 decoder 发出的事件，通知用户 stream 参数变化（如分辨率改变）。携带 `V4L2_EVENT_SRC_CH_RESOLUTION`。

## 工作流程

**Drain**:
确保所有已送入 OUTPUT 的 buffer 都被完全解码，将 CAPTURE 中残留的帧排出。通过 `VIDIOC_DECODER_CMD(V4L2_DEC_CMD_STOP)` 触发。

**DRC (Dynamic Resolution Change)**:
解码过程中码流分辨率动态变化时，decoder 触发的事件序列，需重新配置 CAPTURE queue。

**Seek**:
调至码流中任意位置开始解码。需要停止 OUTPUT queue，再重新启动。
_Avoid_: seeking

**EOS (End of Stream)**:
码流结束标记。触发与 drain 相同的处理流程。
