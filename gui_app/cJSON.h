#ifndef GUI_APP_CJSON_H
#define GUI_APP_CJSON_H

#include <stdbool.h>
#include <json-c/json.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    cJSON_Invalid = 0,
    cJSON_Object,
    cJSON_Array,
    cJSON_String,
    cJSON_Number,
    cJSON_Bool,
    cJSON_NULL
};

typedef struct cJSON cJSON;

struct cJSON {
    struct json_object *object;
    int type;
    int valueint;
    double valuedouble;
    const char *valuestring;
    struct cJSON *first_child;
    struct cJSON *next;
    bool owns_reference;
};

cJSON *cJSON_Parse(const char *value);
void cJSON_Delete(cJSON *item);
cJSON *cJSON_GetObjectItemCaseSensitive(const cJSON *object, const char *string);
int cJSON_IsNumber(const cJSON *item);
int cJSON_IsString(const cJSON *item);
int cJSON_IsObject(const cJSON *item);
int cJSON_IsArray(const cJSON *item);

#ifdef __cplusplus
}
#endif

#endif /* GUI_APP_CJSON_H */
