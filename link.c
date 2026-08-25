#include "link.h"
#include <stdlib.h>
#include <stdio.h>
#include <json/json.h>
#include <string.h>


Node *g_music_head = NULL;

int init_link()
{
    g_music_head = (Node *)malloc(sizeof(Node));
    if (NULL == g_music_head)
    {
        perror("malloc");
        return -1;
    }

    g_music_head->next = NULL;
    g_music_head->prior = NULL;

    return 0;
}


// 解析json字符串，把音乐数据放到链表里面
int link_create_list(const char *s)
{
    struct json_object *obj = json_tokener_parse(s);
    if (NULL == obj)
    {
        fprintf(stderr, "[ERROR] 不是json对象");
        return -1;
    }

    struct json_object *val = json_object_object_get(obj, "cmd");
    if (strcmp("reply_music", json_object_get_string(val)))
    {
        fprintf(stderr, "[ERROR] json格式不对");
        return -1;
    }

    struct json_object *array = json_object_object_get(obj, "music");

    for (int i = 0; i < json_object_array_length(array); i++)
    {
        struct json_object *music = json_object_array_get_idx(array, i);

        // 把music对象转换成字符串并插入链表
        if (link_insert_elem(json_object_get_string(music)) == -1)
        {
            fprintf(stderr, "[ERROR] 元素插入失败");
            continue;
        }
    }

    json_object_put(obj);

    return 0;
}


int link_insert_elem(const char *name)
{
    Node *p = g_music_head;
    while (p->next)
        p = p->next;
    
    Node *new = malloc(sizeof(Node));
    if (NULL == new)
    {
        return -1;
    }

    strcpy(new->music_name, name);
    new->next = NULL;
    new->prior = p;
    p->next = new;

    return 0;
}


void link_traverse_list()
{
    Node *p = g_music_head->next;

    while(p)
    {
        printf("%s", p->music_name);
        p = p->next;
    }
    printf("\n");
}
