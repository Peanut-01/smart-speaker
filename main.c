#include <stdio.h>
#include "select.h"
#include "link.h"
#include "player.h"
#include <stdlib.h>
#include <signal.h>
#include "socket.h"
#include "device.h"
#include "main.h"
#include "socket.h"


int main()
{
    // 运行初始化脚本
    system("/home/music_player/init.sh");

    signal(SIGUSR1, socket_update_music);

    // 初始化集合   select
    if (init_select() != 0)
    {
        printf("集合初始化失败\n");
        return -1;
    }
    printf("集合初始化成功\n");
    // 初始化链表
    if (init_link() == -1)
    {
        printf("链表初始化失败\n");
        return -1;
    }
    printf("链表初始化成功\n");

    // 初始化共享内存
    if (init_shm() == -1)
    {
        printf("共享内存初始化失败\n");
        return -1;
    }
    printf("共享内存初始化成功\n");

    // 初始化音量
    device_set_volume(DEF_VOLUME);

    // 初始化网络
    if (init_socket() == -1)
    {
        printf("网络初始化失败\n");
        return -1;
    }
    printf("网络初始化成功\n");

    // 初始化按键

    // 获取音乐文件（歌手 / 名字）
    socket_get_music("其他");

    //link_traverse_list();
    
    // 循环监听
    m_select();

    return 0;
}
