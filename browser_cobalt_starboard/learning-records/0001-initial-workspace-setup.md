# Learning Record 0001: Initial Workspace Setup

## 日期

2026-06-15

## 学了什么

- 建立了 AML Starboard 学习教学工作区
- 明确了学习使命：通过 AML Starboard 代码学习嵌入式媒体平台开发
- 探索了教学区代码结构：
  - `third_party/starboard/aml/` — AML 平台移植代码
  - 上游 `~/Code/cobalt/starboard/` — 完整的 Starboard API 定义
- 梳理了高质量资源列表：官方文档 8 篇、源码仓库 2 个、社区 3 个

## 关键洞察

- AML Starboard 移植位于 `third_party/starboard/aml/`，分为 `shared/`（共享）、`arm/`（32位）、`arm64/`（64位）
- shared/ 下的核心模块：player (15 files)、media (19 files)、widevine (7 files)、audio_sink、microphone、speech、system、window、signal
- AML 有多个 LTS 分支 (23/24/25) 和多种变体 (amlogic/amlogic.rdk/amlogic.weston/skyworth.br)
- 主要开发集中在：视频播放管线、DRM、输入处理、RDK 集成

## 对后续学习的影响

下一课应当从 Starboard 架构概览开始，建立整体认知后再深入 AML 具体实现
