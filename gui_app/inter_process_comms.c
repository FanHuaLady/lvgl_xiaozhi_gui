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

static void ipc_store_state(int state)
{
    pthread_mutex_lock(&data_mutex);
    current_state = state;
    pthread_mutex_unlock(&data_mutex);
}

static void ipc_store_message(const char *message)
{
    if (!message) {
        return;
    }

    pthread_mutex_lock(&data_mutex);

    size_t copy_len = strlen(message);
    if (copy_len >= IPC_MESSAGE_CAPACITY) {
        copy_len = IPC_MESSAGE_CAPACITY - 1;
    }

    memcpy(last_message, message, copy_len);
    last_message[copy_len] = '\0';
    has_new_message = true;

    pthread_mutex_unlock(&data_mutex);
}

static bool ipc_extract_text_field(cJSON *object)
{
    static const char *keys[] = {"text", "message"};
    size_t key_count = sizeof(keys) / sizeof(keys[0]);

    for (size_t i = 0; i < key_count; ++i) {
        cJSON *item = cJSON_GetObjectItemCaseSensitive(object, keys[i]);
        if (cJSON_IsString(item) && item->valuestring && item->valuestring[0] != '\0') {
            ipc_store_message(item->valuestring);
            return true;
        }
    }

    return false;
}

static void ipc_handle_payload(cJSON *json)
{
    if (!json) {
        return;
    }

    cJSON *state_item = cJSON_GetObjectItemCaseSensitive(json, "state");
    if (cJSON_IsNumber(state_item)) {
        ipc_store_state(state_item->valueint);
    }

    cJSON *type_item = cJSON_GetObjectItemCaseSensitive(json, "type");
    const char *type_value = NULL;
    if (cJSON_IsString(type_item)) {
        type_value = type_item->valuestring;
    }

    if (type_value && strcmp(type_value, "activation") == 0) {
        cJSON *code_item = cJSON_GetObjectItemCaseSensitive(json, "code");
        if (cJSON_IsString(code_item) && code_item->valuestring) {
            char formatted[IPC_MESSAGE_CAPACITY];
            snprintf(formatted, sizeof(formatted), "Code: %s", code_item->valuestring);
            ipc_store_message(formatted);
            return;
        }
    }

    if (ipc_extract_text_field(json)) {
        return;
    }

    cJSON *payload_item = cJSON_GetObjectItemCaseSensitive(json, "payload");
    if (cJSON_IsObject(payload_item)) {
        if (ipc_extract_text_field(payload_item)) {
            return;
        }

        cJSON *nested_state = cJSON_GetObjectItemCaseSensitive(payload_item, "state");
        if (cJSON_IsNumber(nested_state)) {
            ipc_store_state(nested_state->valueint);
        }
    }

    cJSON *data_item = cJSON_GetObjectItemCaseSensitive(json, "data");
    if (cJSON_IsObject(data_item)) {
        if (ipc_extract_text_field(data_item)) {
            return;
        }
    }
}

static void *ipc_udp_receive_thread(void *arg)
{
    ipc_thread_args_t *args = (ipc_thread_args_t *)arg;
    int port = args ? args->port : 0;
    free(args);

    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0) {
        fprintf(stderr, "[IPC] Failed to create UDP socket: %s\n", strerror(errno));
        pthread_mutex_lock(&data_mutex);
        receiver_started = false;
        pthread_mutex_unlock(&data_mutex);
        return NULL;
    }

    int reuse = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        fprintf(stderr, "[IPC] Failed to set SO_REUSEADDR: %s\n", strerror(errno));
    }

#ifdef SO_REUSEPORT
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) < 0) {
        fprintf(stderr, "[IPC] Failed to set SO_REUSEPORT: %s\n", strerror(errno));
    }
#endif

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons((uint16_t)port);

    if (bind(socket_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        fprintf(stderr, "[IPC] Failed to bind UDP socket (port %d): %s\n", port, strerror(errno));
        close(socket_fd);
        pthread_mutex_lock(&data_mutex);
        receiver_started = false;
        pthread_mutex_unlock(&data_mutex);
        return NULL;
    }

    fprintf(stdout, "[IPC] GUI UDP listener started on port %d\n", port);

    while (1) {
        char buffer[IPC_RX_BUFFER_SIZE];
        struct sockaddr_in client_address;
        socklen_t client_length = sizeof(client_address);

        ssize_t received = recvfrom(socket_fd,
                                    buffer,
                                    sizeof(buffer) - 1,
                                    0,
                                    (struct sockaddr *)&client_address,
                                    &client_length);
        if (received < 0) {
            if (errno == EINTR) {
                continue;
            }
            fprintf(stderr, "[IPC] recvfrom failed: %s\n", strerror(errno));
            usleep(1000);
            continue;
        }

        buffer[received] = '\0';

        cJSON *json = cJSON_Parse(buffer);
        if (!json) {
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
    pthread_mutex_lock(&data_mutex);
    if (receiver_started) {
        pthread_mutex_unlock(&data_mutex);
        return;
    }
    receiver_started = true;
    pthread_mutex_unlock(&data_mutex);

    ipc_thread_args_t *args = malloc(sizeof(*args));
    if (!args) {
        fprintf(stderr, "[IPC] Failed to allocate thread arguments\n");
        pthread_mutex_lock(&data_mutex);
        receiver_started = false;
        pthread_mutex_unlock(&data_mutex);
        return;
    }

    args->port = port;

    pthread_t thread_id;
    int result = pthread_create(&thread_id, NULL, ipc_udp_receive_thread, args);
    if (result != 0) {
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
