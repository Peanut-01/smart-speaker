#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "player.h"
#include <string.h>
#include "link.h"
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>
#include <signal.h>
#include <fcntl.h>
#include <fcntl.h>


int g_shmid = 0;        // 共享内存id
int g_start_flag = 0;   // 是否开始  0-未开始; 1-开始
int g_suspend_flag = 0; // 是否暂停  0-未暂停; 1-暂停
int g_device_mode = ONLINE_MODE; //在线模式

extern Node *g_music_head;

int init_shm()
{
    Shm s = {0};
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
    s.parent_pid = getpid();

    memcpy(addr, &s, sizeof(s));
    shmdt(addr);

    return 0;
}


void parent_get_shm(Shm *s)
{
    // 映射共享内存
    void *addr = shmat(g_shmid, NULL, 0);
    if ((void *)-1 == addr)
    {
        perror("shmat");
        return;
    }
    // 取出共享内存的信息
    memcpy(s, addr, sizeof(Shm));
    shmdt(addr);
}


void parent_set_shm(Shm *s)
{
    // 映射共享内存
    void *addr = shmat(g_shmid, NULL, 0);
    if ((void *)-1 == addr)
    {
        perror("shmat");
        return;
    }
    // 取出共享内存的信息
    memcpy(addr, s, sizeof(Shm));
    shmdt(addr);
}


// 开始播放音乐
void player_start_play() 
{
    if (g_start_flag == 1)
        return;

    char music_name[128] = {0};
    strcpy(music_name, g_music_head->next->music_name);

    g_start_flag = 1;

    player_play_music(music_name);
}


// 用于播放音乐的函数
void player_play_music(char *name) 
{
    pid_t pid = fork();
    if (-1 == pid)
    {
        perror("PARENT FORK");
        return;
    }
    else if (0 == pid)
    {   
        // 子进程
        signal(SIGUSR1, child_quit_process);
        child_process(name);
        exit(0);
    } 
    else
    {
        return;
    }
}


void child_quit_process(int sig)
{
    g_start_flag = 0;   // 修改子进程的标志位
}


// 子进程的播放音乐
void child_process(char *name)
{
    while (g_start_flag)
    {
        pid_t pid = fork();
        if (-1 == pid)
        {
            perror("CHILD FORK");
            break;
        }
        else if (0 == pid)  // 孙进程
        {
            // 读取共享内存
            Shm s;
            parent_get_shm(&s);

            if (strlen(name) == 0)  // 不是第一次进来
            {
                if (link_find_next(s.cur_mode, s.cur_music, name) == -1)    // 播放完了所有歌曲
                {
                    /*** 给父子进程发送信号，
                     * 父进程收到信号：请求新的歌曲并更新
                     * 子进程收到信号，修改标志位
                     * */ 
                    kill(s.parent_pid, SIGUSR1);
                    kill(s.child_pid, SIGUSR1);

                    usleep(100000);
                    exit(0);
                }
            }
            // 修改共享内存

            s.child_pid = getppid();
            s.grand_pid = getpid();
            if (g_device_mode == ONLINE_MODE)
            {
                const char *p = name;
                while (*p != '/')
                    p++;

                strncpy(s.cur_singer, name, p - name);
                strcpy(s.cur_music, p + 1);
            }

            parent_set_shm(&s);

            char music_path[128] = {0};

            if (g_device_mode == ONLINE_MODE)
                strcpy(music_path, ONLINE_URL);

            strcat(music_path, name);

            char *arg[7] = {0};
            arg[0] = "mplayer";
            arg[1] = music_path;
            arg[2] = "-slave";
            arg[3] = "-quiet";
            arg[4] = "-input";
            arg[5] = "file=/home/fifo/cmd_fifo";

            if (execv("/usr/bin/mplayer", arg) == -1)
            {
                fprintf(stderr, "[ERROR] MPLAYER启动失败");
            }
        } 
        else                // 子进程
        {   
            // 清空子进程的name数组
            memset(name, 0, strlen(name));
            int status;
            wait(&status);
        }
    }
}


int write_fifo(const char *cmd)
{
    int fd = open("/home/fifo/cmd_fifo", O_WRONLY);
    if (-1 == fd)
    {
        perror("OPEN FIFO");
        return -1;
    }

    if (write(fd, cmd, strlen(cmd)) == -1)
    {
        perror("WRITE FIFO");
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}


// 结束播放
void player_stop_play() 
{
    // 通知子进程结束
    Shm s;
    parent_get_shm(&s);
    kill(s.child_pid, SIGUSR1);

    // 结束mplayer
    write_fifo("quit\n");

    // 回收子进程资源
    int status;
    waitpid(s.child_pid, &status, 0);

    // 修改标志位
    g_start_flag = 0;
    g_suspend_flag = 0;
}


// 暂停播放
void player_suspend_play()
{
    if (g_start_flag == 0 || g_suspend_flag == 1)
        return;
    
    write_fifo("pause\n");
    g_suspend_flag = 1;
    printf("-------暂停播放--------");
}


// 继续播放
void player_continue_play()
{
    if (g_start_flag == 0 || g_suspend_flag == 0)
        return;
    
    write_fifo("pause\n");
    g_suspend_flag = 0;
    printf("-------继续播放--------");
}

