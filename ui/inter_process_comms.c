#include "inter_process_comms.h"

#include "cJSON.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define IPC_RX_BUFFER_SIZE 2048
#define IPC_MESSAGE_CAPACITY 256

typedef struct {
    int port;
} ipc_thread_args_t;

static pthread_mutex_t data_mutex = PTHREAD_MUTEX_INITIALIZER;
static int current_state = UI_STATE_UNKNOWN;
static char last_message[IPC_MESSAGE_CAPACITY] = {0};
static bool has_new_message = false;
static bool receiver_started = false;

// 保存一个整型状态值到全局变量 current_state
static void ipc_store_state(int state)
{
    pthread_mutex_lock(&data_mutex);
    current_state = state;
    pthread_mutex_unlock(&data_mutex);
}

static void ipc_store_message(const char *message)
{
    if (!message) 
    {
        return;
    }

    pthread_mutex_lock(&data_mutex);

    size_t copy_len = strlen(message);
    if (copy_len >= IPC_MESSAGE_CAPACITY) 
    {
        copy_len = IPC_MESSAGE_CAPACITY - 1;
    }

    memcpy(last_message, message, copy_len);                                    // 将传入的消息复制到全局变量 last_message 中
    last_message[copy_len] = '\0';
    has_new_message = true;                                                     // 标志有新消息

    pthread_mutex_unlock(&data_mutex);
}

// 辅助函数
// 在给定的 JSON 对象中查找常见的文本字段（如 "text" 或 "message"）
// 如果找到有效的字符串字段，则将其存储为最新消息并返回 true；否则返回 false
static bool ipc_extract_text_field(cJSON *object)
{
    static const char *keys[] = {"text", "message"};
    size_t key_count = sizeof(keys) / sizeof(keys[0]);

    for (size_t i = 0; i < key_count; ++i) 
    {
        cJSON *item = cJSON_GetObjectItemCaseSensitive(object, keys[i]);
        if (cJSON_IsString(item) && item->valuestring && item->valuestring[0] != '\0') 
        {
            ipc_store_message(item->valuestring);
            return true;
        }
    }

    return false;
}

// 处理接收到的 JSON 对象，提取相关信息并更新状态或消息
static void ipc_handle_payload(cJSON *json)
{
    if (!json) 
    {
        return;
    }

    cJSON *state_item = cJSON_GetObjectItemCaseSensitive(json, "state");        // 尝试从 JSON 对象中获取 "state" 字段
    if (cJSON_IsNumber(state_item))                                             // 如果 "state" 字段存在且是一个数字
    {
        ipc_store_state(state_item->valueint);                                  // 将数值状态存储到全局变量中
    }

    cJSON *type_item = cJSON_GetObjectItemCaseSensitive(json, "type");
    const char *type_value = NULL;
    if (cJSON_IsString(type_item)) 
    {
        type_value = type_item->valuestring;                                    // 保存到type_value
    }

    if (type_value && strcmp(type_value, "activation") == 0)                    // 如果 "type" 为 "activation"
    {
        cJSON *code_item = cJSON_GetObjectItemCaseSensitive(json, "code");
        if (cJSON_IsString(code_item) && code_item->valuestring) 
        {
            char formatted[IPC_MESSAGE_CAPACITY];
            snprintf(formatted, sizeof(formatted), "Code: %s", code_item->valuestring);
            ipc_store_message(formatted);
            return;
        }
    }

    if (ipc_extract_text_field(json)) 
    {
        return;
    }

    cJSON *payload_item = cJSON_GetObjectItemCaseSensitive(json, "payload");
    if (cJSON_IsObject(payload_item)) 
    {
        if (ipc_extract_text_field(payload_item)) 
        {
            return;
        }

        cJSON *nested_state = cJSON_GetObjectItemCaseSensitive(payload_item, "state");
        if (cJSON_IsNumber(nested_state)) 
        {
            ipc_store_state(nested_state->valueint);
        }
    }

    cJSON *data_item = cJSON_GetObjectItemCaseSensitive(json, "data");
    if (cJSON_IsObject(data_item)) 
    {
        if (ipc_extract_text_field(data_item)) 
        {
            return;
        }
    }
}

// 负责在后台接收 UDP 消息并处理 JSON 格式的 IPC 命令
static void *ipc_udp_receive_thread(void *arg)
{
    // 从线程参数中获取端口号，并设置 up receiver_started 标志以防止重复启动
    ipc_thread_args_t *args = (ipc_thread_args_t *)arg;                                 // 取出线程参数结构体
    int port = args ? args->port : 0;                                                   // 获取端口号
    free(args);

    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);                                     // 创建一个 UDP socket
    if (socket_fd < 0)                                                                  // 创建失败
    {
        fprintf(stderr, "[IPC] Failed to create UDP socket: %s\n", strerror(errno));
        pthread_mutex_lock(&data_mutex);
        receiver_started = false;                                                       // 标记接收线程未启动
        pthread_mutex_unlock(&data_mutex);
        return NULL;
    }

    int reuse = 1;                                                                      // 设置 SO_REUSEADDR 以允许快速重启
    
    // setsockopt 是一个用于设置套接字选项的系统调用（或库函数）
    // 它允许程序在套接字（socket）上配置各种行为属性
    // 从而控制协议栈如何处理该套接字的数据传输
    // 它通常用于调整网络通信的细节，例如地址重用、超时、缓冲区大小、广播权限等。

    // SO_REUSEADDR选项，作用是允许重用本地地址和端口
    // 当一个套接字绑定到一个特定的地址和端口后，默认情况下，操作系统会将该地址和端口标记为占用状态，
    // 直到套接字被完全关闭（包括所有相关的连接都被终止）
    // 这可能会导致在某些情况下无法立即重新绑定到同一地址和端口，例如在服务器重启时
    // 通过设置SO_REUSEADDR选项，程序可以告诉操作系统允许重用已经被占用的地址和端口
    // 这对于服务器应用程序非常有用，因为它允许服务器在重启后立即重新绑定到同一地址和端口，而不必等待
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) 
    {
        fprintf(stderr, "[IPC] Failed to set SO_REUSEADDR: %s\n", strerror(errno));
    }

#ifdef SO_REUSEPORT
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) < 0) {
        fprintf(stderr, "[IPC] Failed to set SO_REUSEPORT: %s\n", strerror(errno));
    }
#endif

    // 定义并清零地址结构体
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;                                                       // 设置地址族为 IPv4
    address.sin_addr.s_addr = htonl(INADDR_ANY);                                        // 绑定到所有可用的网络接口
    // htons 将端口号（16 位整数）从主机字节序转换为网络字节序 
    address.sin_port = htons((uint16_t)port);                                           // 设置端口号

    // bind 函数将套接字 socket_fd 与指定的本地地址（IP + 端口）关联起来
    if (bind(socket_fd, (struct sockaddr *)&address, sizeof(address)) < 0) 
    {
        fprintf(stderr, "[IPC] Failed to bind UDP socket (port %d): %s\n", port, strerror(errno));
        close(socket_fd);
        pthread_mutex_lock(&data_mutex);
        receiver_started = false;
        pthread_mutex_unlock(&data_mutex);
        return NULL;
    }

    fprintf(stdout, "[IPC] GUI UDP listener started on port %d\n", port);

    while (1) 
    {
        char buffer[IPC_RX_BUFFER_SIZE];                                                // 用于存放接收到的数据
        struct sockaddr_in client_address;                                              // 保存发送方的地址信息
        socklen_t client_length = sizeof(client_address);

        // 接收数据
        ssize_t received = recvfrom(socket_fd,
                                    buffer,
                                    sizeof(buffer) - 1,
                                    0,
                                    (struct sockaddr *)&client_address,
                                    &client_length);
        if (received < 0) 
        {
            if (errno == EINTR) 
            {
                continue;
            }
            fprintf(stderr, "[IPC] recvfrom failed: %s\n", strerror(errno));
            usleep(1000);
            continue;
        }

        buffer[received] = '\0';

        // 使用 cJSON 库 将字符串解析为 JSON 对象
        cJSON *json = cJSON_Parse(buffer);
        if (!json) 
        {
            fprintf(stderr, "[IPC] Unable to parse incoming JSON: %s\n", buffer);
            continue;
        }

        ipc_handle_payload(json);
        cJSON_Delete(json);
    }

    close(socket_fd);

    pthread_mutex_lock(&data_mutex);
    receiver_started = false;
    pthread_mutex_unlock(&data_mutex);

    return NULL;
}

void ipc_init(int port)
{
    pthread_mutex_lock(&data_mutex);                                        // 互斥锁 保护receiver_started
    if (receiver_started)                                                   // 指示 UDP 接收线程是否已经启动，防止重复启动
    {
        pthread_mutex_unlock(&data_mutex);                                  // 解锁
        return;
    }
    receiver_started = true;                                                // 标记接收线程已启动
    pthread_mutex_unlock(&data_mutex);                                      // 解锁 

    ipc_thread_args_t *args = malloc(sizeof(*args));                        // 动态分配一个 ipc_thread_args_t 结构体
    if (!args)                                                              // 分配失败
    {
        fprintf(stderr, "[IPC] Failed to allocate thread arguments\n");
        pthread_mutex_lock(&data_mutex);
        receiver_started = false;
        pthread_mutex_unlock(&data_mutex);
        return;
    }

    args->port = port;                                                      // 将传入的端口号存储在结构体中    

    pthread_t thread_id;
    int result = pthread_create(&thread_id, NULL, ipc_udp_receive_thread, args);                // 创建线程
    if (result != 0) 
    {
        fprintf(stderr, "[IPC] Failed to create UDP listener thread: %s\n", strerror(result));
        free(args);
        pthread_mutex_lock(&data_mutex);
        receiver_started = false;
        pthread_mutex_unlock(&data_mutex);
        return;
    }

    pthread_detach(thread_id);
}

int ipc_get_state(void)
{
    pthread_mutex_lock(&data_mutex);
    int state = current_state;
    pthread_mutex_unlock(&data_mutex);
    return state;
}

bool ipc_get_latest_message(char *buffer, size_t len)
{
    if (!buffer || len == 0) {
        return false;
    }

    pthread_mutex_lock(&data_mutex);

    if (!has_new_message) {
        pthread_mutex_unlock(&data_mutex);
        buffer[0] = '\0';
        return false;
    }

    size_t copy_len = strlen(last_message);
    if (copy_len >= len) {
        copy_len = len - 1;
    }
    memcpy(buffer, last_message, copy_len);
    buffer[copy_len] = '\0';
    has_new_message = false;

    pthread_mutex_unlock(&data_mutex);
    return true;
}
