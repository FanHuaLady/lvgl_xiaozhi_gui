# Xiaozhi AI LVGL GUI

本项目是为 [xiaozhi_linux_rs](https://github.com/Hyrsoft/xiaozhi_linux_rs) 提供的一个示例 GUI 客户端实现。它基于嵌入式 UI 框架 [LVGL](https://github.com/lvgl/lvgl) 构建，适配 **320×240** 分辨率的 Linux Framebuffer (`/dev/fb0`) 设备（例如 Luckfox Pico / RV1106 系列开发板）。

与音频/唤醒核心进程通过 **UDP Socket** 通信，实现状态的实时可视化。

## 功能特性

- 动态显示设备不同工作状态：
  - 待机空闲 (Online)
  - 离线/断开 (Disconnected)
  - 正在倾听 (Listening)
  - 正在思考 (Thinking)
  - 正在说话 (Speaking)
- 实时显示 TTS 云端播报的对话框文本。
- 接收并展示 Toast 通知消息和配网激活码。
- 支持触摸/点击事件。

## 编译前配置

在编译之前，请根据实际环境修改以下配置：

### 1. 通信端口配置 (`main.c`)

GUI 默认监听 UDP 端口 **5679**，接收来自 Core 进程发送的状态、TTS 文本等信息。
在 `main.c` 中找到以下代码进行修改：

```c
const char *ipc_port_env = getenv("GUI_IPC_PORT");
int ipc_port = ipc_port_env ? atoi(ipc_port_env) : 5679;
```

你也可以在运行时通过环境变量动态指定端口，无需重新编译：

```bash
GUI_IPC_PORT=5679 ./xiaozhi_linux_lvgl
```

> **注意：** 请确保 Core 进程（`xiaozhi_linux_rs`）的目标地址和端口与此处设置一致。

### 2. 字体文件路径 (`ui/fonts/freetype_fonts.c`)

项目使用 FreeType 在运行时加载字体文件，默认路径为：

```c
static const char* kFontFallbacks[] = {
    "/root/SourceHanSansSC-Regular.otf",
};
```

请从以下地址下载思源黑体字体文件，并将其上传到目标设备的 `/root/` 目录：

```
https://github.com/Hyrsoft/xiaozhi_linux_echo_mate/releases/download/v1.0.0/SourceHanSansSC-Regular.otf
```

如需修改字体路径，编辑 `ui/fonts/freetype_fonts.c` 中的 `kFontFallbacks` 数组即可。

### 3. 工具链路径 (`cmake/toolchain-luckfox-pico.cmake`)

根据本机 SDK 的实际安装路径修改 `cmake/toolchain-luckfox-pico.cmake`：

```cmake
set(SDK_PATH "/project/SDK/rv1106-sdk")
```

将 `SDK_PATH` 替换为你的 SDK 实际路径，例如 `/home/you/Echo-Mate/SDK/rv1106-sdk`。

## 交叉编译步骤

### 1. 安装依赖（宿主机）

```bash
sudo apt update
sudo apt install ninja-build cmake git
```

### 2. 初始化 LVGL 子模块

```bash
git submodule update --init --recursive
```

### 3. 在 Buildroot 中启用 FreeType

FreeType 库需要预先在 Buildroot 的 sysroot 中编译安装，否则链接阶段会报错。

```bash
cd /path/to/rv1106-sdk/sysdrv/source/buildroot/buildroot-2023.02.6
make menuconfig
```

在 menuconfig 中导航到：

```
Target packages → Libraries → Graphics → [*] freetype
```

保存配置后重新编译 Buildroot：

```bash
make
```

### 4. 配置工具链路径

```bash
vim cmake/toolchain-luckfox-pico.cmake
```

将 `SDK_PATH` 修改为你的 SDK 实际安装路径。

### 5. 构建编译

**Debug 构建：**

```bash
cmake --preset Debug -DUI_USE_SIMULATOR=OFF
cmake --build --preset Debug
```

**Release 构建：**

```bash
cmake --preset Release -DUI_USE_SIMULATOR=OFF
cmake --build --preset Release
```

编译产物位于 `build/Debug/` 或 `build/Release/` 目录下，可执行文件名为 `xiaozhi_linux_lvgl`。

### 6. 部署到目标设备

将编译产物和字体文件拷贝到目标设备：

```bash
# 拷贝可执行文件
scp build/Debug/xiaozhi_linux_lvgl root@<设备IP>:/root/

# 下载并拷贝字体文件
wget https://github.com/Hyrsoft/xiaozhi_linux_echo_mate/releases/download/v1.0.0/SourceHanSansSC-Regular.otf
scp SourceHanSansSC-Regular.otf root@<设备IP>:/root/
```

## 运行

在目标设备上先启动 Core 进程，再启动 GUI：

```bash
# 启动 Core 进程
./xiaozhi_linux_rs-armv7-uclibceabihf

# 启动 GUI（默认使用 /dev/fb0 和 UDP 5679 端口）
./xiaozhi_linux_lvgl
```

如需自定义 Framebuffer 设备或端口：

```bash
LV_LINUX_FBDEV_DEVICE=/dev/fb0 GUI_IPC_PORT=5679 ./xiaozhi_linux_lvgl
```

## 注意事项

- 本项目适配 **320×240** 分辨率的 Linux Framebuffer 设备（`/dev/fb0`），使用 RGB565 颜色格式。
- 触摸输入设备默认为 `/dev/input/event0`，可根据实际设备修改 `main.c` 中 `lv_evdev_create` 的参数。
- 字体文件必须在运行时存在于目标设备的指定路径，否则中文字符将无法正常显示。

## 协议

本项目基于 **GPL-3.0** 许可证开源。请参阅项目根目录下的 [LICENSE](LICENSE) 文件了解更多详情。
