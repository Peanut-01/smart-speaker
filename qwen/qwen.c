#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <json/json.h>
#include <stdlib.h>


void parse_json(const char *json, char *content)
{
    // 字符串转化成json对象
    struct json_object *obj = json_tokener_parse(json);
    if (obj == NULL)
    {
        fprintf(stderr, "json解析失败\n");
        return;
    }

    // 1.获取choices数组
    struct json_object *choice_arr = NULL;
    choice_arr = json_object_object_get(obj, "choices");

    if (choice_arr == NULL || json_object_get_type(choice_arr) != json_type_array)
    {
        fprintf(stderr, "不是一个数组对象\n");
        json_object_put(obj);
        return;
    }

    // 获取数组第0个元素（message）
    struct json_object *first_choice = NULL;
    first_choice = json_object_array_get_idx(choice_arr, 0);

    struct json_object *message_obj = NULL;
    message_obj = json_object_object_get(first_choice, "message");

    struct json_object *content_obj = NULL;
    content_obj = json_object_object_get(message_obj, "content");

    strcpy(content, json_object_get_string(content_obj));

    json_object_put(obj);

    if (strlen(content) > 0)
    {
        printf("--> %s\n", content);
    }
}


int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "请加上你的问题\n");
        return -1;
    }

    char command[1024] = {0};
    sprintf(command, "/home/qwen/qwen.sh %s 2>/dev/null", argv[1]);
    // sprintf(command, "/home/smart_speaker/qwen/qwen.sh %s 2>/dev/null", argv[1]);

    FILE *fp = popen(command, "r");
    if (fp == NULL)
    {
        perror("popen");
        return -1;
    }

    char buf[2048] = {0};
    fgets(buf, sizeof(buf), fp);
    printf("--> %s\n", buf);

    pclose(fp);

    char content[1024] = {0};
    parse_json(buf, content);

    return 0;
}