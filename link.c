#include "link.h"
#include <stdlib.h>
#include <stdio.h>
#include <json/json.h>
#include <string.h>
#include "player.h"


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


// 插入元素
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


// 遍历链表并打印
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


// 根据传入的参数找到下一首歌，如果找不到下一首返回-1，找到返回0
int link_find_next(int mode, char *cur, char *next)
{
    if (NULL == cur || NULL == next)
        return -1;
    if (mode == CIRCLE)
    {
        strcpy(next, cur);
        return 0;
    }

    Node *p = g_music_head->next;

    // 寻找当前的歌
    while (p)
    {
        if (strstr(p->music_name, cur))
            break;
        
        p = p->next;    
    }

    if (p->next == NULL)    // 已经是最后一首歌
        return -1;
    
    strcpy(next, p->next->music_name);
    return 0;
}


// 清空链表
void link_clear_list()
{
    Node *p = g_music_head->next;
    while (p)
    {
        Node *q = p->next;
        free(p);
        p = q;
    }

    g_music_head->next = NULL;
    
}


// 根据当前歌曲找到上一首
void link_find_prior(const char *cur, char *music)
{
    if (NULL == cur || NULL == music)
        return;

    Node *p = g_music_head->next;
    if (strstr(p->music_name, cur))
    {
        // 已经是第一首歌
        strcpy(music, p->music_name);
        return;
    }

    while (p)
    {
        if (strstr(p->music_name, cur)) 
        {
            strcpy(music, p->prior->music_name);
            return;
        }
        p = p->next;
    }
}
