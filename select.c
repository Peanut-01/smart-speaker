#include <sys/select.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "player.h"
#include <pthread.h>


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
    printf("\t9.单曲循环        10.顺序播放\n");
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
            // 临时处理代码
            FD_CLR(g_sockfd, &READSET);
            g_maxfd = (g_maxfd == g_sockfd) ? (g_maxfd - 1) : g_maxfd;
            close(g_sockfd);
            pthread_cancel(tid);
        }
        
    }
    
}
