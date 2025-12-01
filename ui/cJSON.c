#include "cJSON.h"

#include <stdlib.h>
#include <string.h>

static int cjson_determine_type(struct json_object *object)
{
    if (!object) {
        return cJSON_Invalid;
    }

    enum json_type type = json_object_get_type(object);
    switch (type) {
        case json_type_object:
            return cJSON_Object;
        case json_type_array:
            return cJSON_Array;
        case json_type_string:
            return cJSON_String;
        case json_type_int:
        case json_type_double:
            return cJSON_Number;
        case json_type_boolean:
            return cJSON_Bool;
        case json_type_null:
            return cJSON_NULL;
        default:
            break;
    }
    return cJSON_Invalid;
}

static void cjson_populate_node(cJSON *node)
{
    if (!node || !node->object) {
        return;
    }

    node->valuestring = NULL;
    node->valueint = 0;
    node->valuedouble = 0.0;

    switch (node->type) {
        case cJSON_String:
            node->valuestring = json_object_get_string(node->object);
            break;
        case cJSON_Number:
            node->valueint = json_object_get_int(node->object);
            node->valuedouble = json_object_get_double(node->object);
            break;
        case cJSON_Bool:
            node->valueint = json_object_get_boolean(node->object) ? 1 : 0;
            node->valuedouble = (double)node->valueint;
            break;
        default:
            break;
    }
}

static void cjson_free_node(cJSON *node)
{
    if (!node) {
        return;
    }

    cJSON *child = node->first_child;
    while (child) {
        cJSON *next = child->next;
        cjson_free_node(child);
        child = next;
    }

    if (node->object && node->owns_reference) {
        json_object_put(node->object);
    }

    free(node);
}

cJSON *cJSON_Parse(const char *value)
{
    if (!value) {
        return NULL;
    }

    json_object *parsed = json_tokener_parse(value);
    if (!parsed) {
        return NULL;
    }

    cJSON *root = calloc(1, sizeof(*root));
    if (!root) {
        json_object_put(parsed);
        return NULL;
    }

    root->object = parsed;
    root->owns_reference = true;
    root->type = cjson_determine_type(parsed);
    cjson_populate_node(root);
    return root;
}

void cJSON_Delete(cJSON *item)
{
    cjson_free_node(item);
}

cJSON *cJSON_GetObjectItemCaseSensitive(const cJSON *object, const char *string)
{
    if (!object || !object->object || !string) {
        return NULL;
    }

    if (!json_object_is_type(object->object, json_type_object)) {
        return NULL;
    }

    json_object *child_object = NULL;
    if (!json_object_object_get_ex(object->object, string, &child_object)) {
        return NULL;
    }

    cJSON *child = calloc(1, sizeof(*child));
    if (!child) {
        return NULL;
    }

    child->object = json_object_get(child_object);
    child->owns_reference = true;
    child->type = cjson_determine_type(child_object);
    cjson_populate_node(child);

    child->next = ((cJSON *)object)->first_child;
    ((cJSON *)object)->first_child = child;

    return child;
}

int cJSON_IsNumber(const cJSON *item)
{
    return item && item->type == cJSON_Number;
}

int cJSON_IsString(const cJSON *item)
{
    return item && item->type == cJSON_String;
}

int cJSON_IsObject(const cJSON *item)
{
    return item && item->type == cJSON_Object;
}

int cJSON_IsArray(const cJSON *item)
{
    return item && item->type == cJSON_Array;
}
