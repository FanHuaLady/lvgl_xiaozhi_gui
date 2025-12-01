## DeskBot demo

### 1.编译
1. **在电脑上运行SDL仿真**

    使用CMake Preset并开启`UI_USE_SIMULATOR`即可启用SDL后端：
    
    ```sh
    cmake --preset Debug -DUI_USE_SIMULATOR=ON
    cmake --build --preset Debug
    ./build/Debug/main
    ```

2. **编译到开发板上运行**

    默认关闭`UI_USE_SIMULATOR`即为真实硬件配置，可根据需要选择`Debug`或`Release`预设：

    ```sh
    cmake --preset Debug
    cmake --build --preset Debug
    ```

    将生成的`build/Debug`目录整体拷贝到开发板上，然后在该目录中执行`./main`即可运行。

### 2.Server运行

python环境搭建详见[AI语言助手demo(Server端)](../AIChat_demo/Server/README.md), 然后进入你搭好的虚拟环境中，运行即可

``` sh
python ./main.py --access_token="123456"
```

### 3.注意

**!! 应用不再加载本地 system_para.conf / gaode_adcode.json，UI 数据完全通过 IPC 输入 !!**

1. 只需保持 IPC 服务端运行，通过 `inter_process_comms` 发送 JSON 消息即可驱动界面，无需再拷贝或维护任何系统配置文件。

2. 每个page可视为一个APP, 想要自行添加可以参考模版添加

3. 你的个人电脑运行服务端`server`时, 需要跟开发板`Client`端, 连接到同一个网络下, AI chat才能正常访问. 当然, 你也可以使用电脑进行网络共享(默认开发板是172.32.0.93, 电脑是172.32.0.100).