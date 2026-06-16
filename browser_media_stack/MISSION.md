# Learning Mission

## Goal
从 `<video>` 标签到像素上屏，完整理解浏览器视频播放的实现链路，成为一名能够构建 Web 视频播放器产品的 C++ 系统开发者。

## Why
视频是互联网上最重要的内容形式之一。浏览器的视频播放能力已经从简单的 MP4 文件播放演变成一个跨越 JavaScript、渲染引擎、操作系统、硬件解码器、DRM 安全组件等多个 layer 的复杂系统。

## What you already know
- C++ 系统编程
- FFmpeg 和音视频基本概念（封装格式、编码、解码）
- 通用 media pipeline 基础
- Linux 平台开发

## What you'll learn
- 浏览器如何加载和处理视频资源（MSE / progressive download）
- Blink 渲染引擎中 HTMLVideoElement ↔ WebMediaPlayer 的桥接
- Chromium 中 demuxer 层的设计（FFmpegDemuxer / ChunkDemuxer）
- 跨进程硬件解码的抽象（VDA / MojoVideoDecoder）
- Compositor 中的视频帧渲染路径（overlay vs GPU 合成）
- Linux 平台层：dma-buf / Wayland / KMS / GStreamer 集成
- EME / CDM / TEE：Widevine L1 vs L3 的安全视频路径
