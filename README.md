# 小智Linux客户端GUI

**1、没有Ninja，需要下载**

```
sudo apt update
sudo apt install ninja-build
```

**2、设置工具链路径**

```
vim toolchain-luckfox-pico.cmake
```

**3、初始化 LVGL 子模块**

```
git submodule update --init --recursive
```

**4、安装 FreeType 到 sysroot**

找到Target packages→Libraries→Graphics→[*] freetype

```
cd /home/you/Echo-Mate/SDK/rv1106-sdk/sysdrv/source/buildroot/buildroot-2023.02.6
make menuconfig
```

**5、构建编译**

```
cmake --preset Debug -DUI_USE_SIMULATOR=OFF
```

```
cmake --build --preset Debug
```

**6、运行客户端程序和GUI程序**

```
./xiaozhi_linux_rs-armv7-uclibceabih
```

```
./xiaozhi_linux_lvgl
```
