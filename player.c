#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "player.h"
#include <string.h>

int g_shmid = 0;        // 共享内存id
int g_start_flag = 0;   // 是否开始
int g_suspend_flag = 0; // 是否暂停

int init_shm()
{
    Shm s;
    g_shmid = shmget(SHMKEY, SHMSIZE, IPC_EXCL | IPC_CREAT);
    if (-1 == g_shmid) 
    {
        perror("shmget");
        return -1;
    }

    void *addr = shmat(g_shmid, NULL, 0);
    if ((void *)-1 == addr)
    {
        perror("shmat");
        return -1;
    }

    // 初始化共享内存中的数据
    s.cur_mode = SEQUENCE;

    memcpy(addr, &s, sizeof(s));
    shmdt(addr);

    return 0;
}