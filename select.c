#include <sys/select.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <json/json.h>
#include "player.h"
#include "socket.h"


fd_set READSET;
extern int g_maxfd;
extern int g_sockfd;
extern pthread_t tid;


int init_select() 
{
    FD_ZERO(&READSET);
    FD_SET(STDIN_FILENO, &READSET);
    return 0;
}


void show_menu()
{
    system("clear");

    printf("\t1.开始播放        2.结束播放\n");
    printf("\t3.暂停播放        4.继续播放\n");
    printf("\t5.下一首          6.上一首\n");
    printf("\t7.增加音量        8.减小音量\n");
    printf("\t9.单曲循环        a.顺序播放\n");
}


void select_read_stdio()
{
    char ch;
    scanf("%c", &ch);

    printf("[SELECT] 读到键盘数据 [%c]\n", ch);

    getchar();

    switch (ch)
    {
    case '1':
        player_start_play();
        break;
    case '2':
        player_stop_play();
        break;
    case '3':
        player_suspend_play();
        break;
    case '4':
        player_continue_play();
        break;
    case '5':
        player_next_play();
        break;
    case '6':
        player_prior_play();
        break;
    case '7':
        player_volumn_up();
        break;
    case '8':
        player_volumn_down();
        break;
    case '9':
        player_set_mode(CIRCLE);
        break;
    case 'a':
        player_set_mode(SEQUENCE);
        break;
    }
}


// 解析数据
void parse_message(const char *buf, char *cmd)
{
    struct json_object *obj = json_tokener_parse(buf);
    if (NULL == obj)
    {
        fprintf(stderr, "[ERROR] 不是一个json格式\n");
        return;
    }

    struct json_object *value;
    value = json_object_object_get(obj, "cmd");
    if (NULL == value)
    {
        fprintf(stderr, "[ERROR] 没有包含 cmd 字段\n");
        json_object_put(obj);
        return;
    }

    strcpy(cmd, json_object_get_string(value));
    json_object_put(obj);

}


// 读取数据
void select_read_socket()
{
    char buf[1024] = {0};
    char cmd[32] = {0};

    socket_recv_data(buf);
    parse_message(buf, cmd);

    if (!strcmp(cmd, "app_start"))
    {
        socket_start_play();
    }
    else if (!strcmp(cmd, "app_stop"))
    {
        socket_stop_play();
    }
    else if (!strcmp(cmd, "app_suspend"))
    {
        socket_suspend_play();
    }
    else if (!strcmp(cmd, "app_continue"))
    {
        socket_continue_play();
    }
    else if (!strcmp(cmd, "app_next"))
    {
        socket_next_play();
    }
    else if (!strcmp(cmd, "app_prior"))
    {
        socket_prior_play();
    }
    else if (!strcmp(cmd, "app_voice_up"))
    {
        socket_volumn_up();
    }
    else if (!strcmp(cmd, "app_voice_down"))
    {
        socket_volumn_down();
    }
    else if (!strcmp(cmd, "app_circle"))
    {
        socket_set_mode(CIRCLE);
    }
    else if (!strcmp(cmd, "app_sequence"))
    {
        socket_set_mode(SEQUENCE);
    }
    else
    {
        printf("不匹配");
    }
}


void m_select()
{
    fd_set TMPSET;

    show_menu();

    while (1)
    {
        TMPSET = READSET;
        int ret = select(g_maxfd + 1, &TMPSET, NULL, NULL, NULL);
        if (-1 == ret && errno != EINTR)
        {
            perror("select");
            continue;
        }

        if (FD_ISSET(STDIN_FILENO, &TMPSET))   // 键盘有数据可读
        {
            select_read_stdio();
        }
        else if (FD_ISSET(g_sockfd, &TMPSET))   // 网络可读
        {
            select_read_socket();
        }
        
    }
    
}
