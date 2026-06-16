# Resources

## 官方文档

| 资源 | 链接 | 说明 |
|------|------|------|
| Starboard 设计原则 | `~/Code/cobalt/starboard/doc/principles.md` | Starboard 的 11 个目标和 8 个设计原则 |
| Starboard 移植指南 | `~/Code/cobalt/starboard/README.md` | 官方移植快速入门和实现顺序 |
| Starboard ABI | `~/Code/cobalt/starboard/doc/starboard_abi.md` | 二进制兼容性规范 |
| Starboard 版本管理 | `~/Code/cobalt/starboard/doc/versioning.md` | API 版本与冻结策略 |
| Evergreen 概述 | `~/Code/cobalt/starboard/doc/evergreen/cobalt_evergreen_overview.md` | 云端可更新架构 |
| Starboard 16 POSIX 迁移 | `~/Code/cobalt/starboard/doc/starboard_16_posix.md` | 废弃 API 的 POSIX 替代方案 |
| Starboard 代码风格 | `~/Code/cobalt/starboard/doc/style.md` | Starboard API 命名和代码规范 |
| Build 系统迁移 | `~/Code/cobalt/starboard/build/doc/migration_changes.md` | GYP 到 GN 的迁移 |

## 源码

| 仓库 | 路径 | 说明 |
|------|------|------|
| 上游 Cobalt | `~/Code/cobalt/` | 完整的 Cobalt + Starboard 上游代码 |
| AML Starboard Fork | `/home/teng.wang/Teach/cobalt/starboard/` | Amlogic 的 Starboard 移植仓库 |
| AML Starboard 移植代码 | `third_party/starboard/aml/` | AML 平台具体实现 (arm, arm64, shared) |

## 社区

| 社区 | 链接 | 说明 |
|------|------|------|
| Cobalt 官方讨论组 | https://groups.google.com/g/cobalt-dev | Google Groups |
| RDK 社区 | https://rdkcentral.com/ | RDK (Reference Design Kit) |
| Amlogic 开发者资源 | https://www.amlogic.com/ | 官方开发者门户 |

## 关键知识领域

- **嵌入式 Linux (Yocto)**: AML 基于 Yocto 构建
- **GStreamer**: AML RDK 移植的媒体管线基于 GStreamer
- **Widevine DRM**: 内容保护方案
- **ESSOS**: AML 的图形/输入系统 (libessos)
- **OpenWF / Westeros**: 显示合成器
- **V4L2**: Video4Linux2，AML 视频编码通过 V4L2 接口
