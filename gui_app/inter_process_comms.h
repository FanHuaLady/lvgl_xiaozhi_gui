#ifndef GUI_APP_INTER_PROCESS_COMMS_H
#define GUI_APP_INTER_PROCESS_COMMS_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    UI_STATE_UNKNOWN = 0,
    UI_STATE_IDLE = 3,
    UI_STATE_LISTENING = 4,
    UI_STATE_THINKING = 5,
    UI_STATE_SPEAKING = 6,
    UI_STATE_ERROR = 9
} ui_state_t;

void ipc_init(int port);
int ipc_get_state(void);
bool ipc_get_latest_message(char *buffer, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* GUI_APP_INTER_PROCESS_COMMS_H */
