# EPlayerClient
Qt6 加密播放器客户端，配套验证服务器为 [EPlayer](https://github.com/GanYv123/EPlayer)。

## 项目组成

- `loginform.*`：登录界面与服务器验证请求。
- `widget.*`、`qmediaplayer.*`：主界面及播放器交互。
- `vlchelper.*`、`mediamp4.*`：LibVLC 播放封装与媒体读取。
- `ssltool.*`：加密相关工具。
- `sdk/`：VLC SDK；根目录包含部分 VLC 运行库。

## 构建要求

- Windows、Qt6，以及支持 C++17 的编译器。
- Qt Core、Gui、Widgets、Network 模块。
- OpenSSL 和 VLC / LibVLC，依赖库与 Qt Kit 的架构必须一致；当前工程在 64 位配置中链接 VLC。

## 构建与运行

1. 克隆仓库：`git clone https://github.com/GanYv123/EPlayerClient.git`。
2. 在 Qt Creator 中打开 `EPlayerClient.pro`，选择与依赖库匹配的 64 位 Kit。
3. 按实际安装位置调整 `.pro` 中 OpenSSL 的 `INCLUDEPATH`、`LIBS`；当前配置使用本机 vcpkg 静态库路径 `C:/src/vcpkg/installed/x64-windows-static`。
4. 检查 `sdk/include`、`sdk/lib` 中的 VLC SDK 文件后构建。
5. 将 `loginform.cpp` 中的 `HOST` 改为实际 EPlayer 服务地址，并确认服务器已启动。
6. 运行时准备 Qt 运行库、`libvlc.dll`、`libvlccore.dll` 及匹配版本的 VLC 插件目录；按所用 OpenSSL 链接方式准备依赖。

本仓库保留开发阶段的工程配置，依赖路径和服务地址需要按本机环境调整。
