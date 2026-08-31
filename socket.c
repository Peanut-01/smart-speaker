#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <string.h>
#include "socket.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>
#include <pthread.h>
#include "player.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include "main.h"
#include <json/json.h>
#include "device.h"
#include "link.h"


int g_sockfd = 0;
int g_maxfd = 0;
pthread_t tid;

extern fd_set READSET;
extern int g_start_flag;
extern int g_suspend_flag;
extern int g_device_mode;


void socket_update_music(int sig)
{
    // 修改父进程播放标志位
    g_start_flag = 0;
    // 回收子进程
    Shm s;
    parent_get_shm(&s);
    int status;
    waitpid(s.child_pid, &status, 0);
    // 清空链表
    link_clear_list();
    // 请求新的数据并更新链表
    socket_get_music(s.cur_singer);

    player_start_play();
}


// 封装json并发送到服务器
void socket_send_data(struct json_object *obj)
{
    char buf[1024] = {0};
    int len = 0;

    // 把json对象转化成字符串
    const char *data = json_object_to_json_string(obj);
    if (NULL == data)
    {
        printf("NOT JSON OBJECT");
        return;
    }

    len = strlen(data);
    memcpy(buf, &len, 4);
    memcpy(buf + 4, data, len);

    //printf("发送 %d 字节\n", len);

    if (send(g_sockfd, buf, len + 4, 0) == -1) 
    {
        perror("send server");
        return;
    }
}


void *send_server(void *arg)
{
    while (1)
    {
        // 创建json对象
        struct json_object *obj = json_object_new_object();
        // 添加json键值对，并写到共享内存中
        Shm s;
        memset(&s, 0, sizeof(s));
        parent_get_shm(&s);
        json_object_object_add(obj, "cmd", json_object_new_string("info"));
        json_object_object_add(obj, "cur_music", json_object_new_string(s.cur_music));
        json_object_object_add(obj, "mode", json_object_new_int(s.cur_mode));

        char status[8] = {0};
        if (g_start_flag == 0)
            strcpy(status, "stop");
        if (g_start_flag == 1 && g_suspend_flag == 0)
            strcpy(status, "start");
        if (g_start_flag == 1 && g_suspend_flag == 1)
            strcpy(status, "suspend");

        json_object_object_add(obj, "status", json_object_new_string(status));
        json_object_object_add(obj, "deviceid", json_object_new_string(DEVICEID));

        // 获取当前音量，并封装到json中
        int volume;
        device_get_volume(&volume);
        json_object_object_add(obj, "volumn", json_object_new_int(volume));

        // 发送数据给服务器
        socket_send_data(obj);

        json_object_put(obj);

        sleep(2);
    }
    
}

int init_socket() 
{
    int count = 50;

    g_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == g_sockfd)
    {
        perror("socket");
        return -1;
    }

    // 创建服务端结构体并填入信息
    struct sockaddr_in server_info;
    memset(&server_info, 0, sizeof(server_info));

    server_info.sin_family = AF_INET;
    server_info.sin_port = htons(PORT);
    server_info.sin_addr.s_addr = inet_addr(IP);

    int ret;

    while (count--)
    {
        // 持续连接
        ret = connect(g_sockfd, (struct sockaddr *)&server_info, sizeof(server_info));
        if (-1 == ret)
        {
            perror("CONNECT SERVER FAILURE");
            sleep(1);
            continue;
        }
        // 连接成功，把当前客户端 fd 加入到集合中
        FD_SET(g_sockfd, &READSET);
        g_maxfd = (g_maxfd > g_sockfd) ? g_maxfd : g_sockfd;

        // 每隔5s上传数据（当前歌曲 模式 音量 状态）
        if (pthread_create(&tid, NULL, send_server, NULL) != 0)
        {
            perror("pthread_create");
            return -1;
        }

        // 切换为在线模式
        g_device_mode = ONLINE_MODE;

        return 0;
    }

    return -1;
}


void socket_get_music(const char *singer)
{
    // 创建json对象
    struct json_object *obj = json_object_new_object();
    // 添加json键值对
    json_object_object_add(obj, "cmd", json_object_new_string("get_music_list"));
    json_object_object_add(obj, "singer", json_object_new_string(singer));

    // 向服务器请求数据
    socket_send_data(obj);

    // 等待服务器返回
    char msg[1024] = {0};
    socket_recv_data(msg);

    // 把音乐数据插入链表
    link_create_list(msg);

    // 释放json对象
    json_object_put(obj);
}


// 接收服务端的信息
void socket_recv_data(char *msg)
{
    int len = 0;
    size_t size = 0;
    while (1)
    {
        size += recv(g_sockfd, msg + size, sizeof(int) - size, 0);
        if (size >= sizeof(int))
            break;
        else if (0 == size)
        {
            FD_CLR(g_sockfd, &READSET);
            g_maxfd = (g_maxfd == g_sockfd) ? (g_maxfd - 1) : g_maxfd;
            close(g_sockfd);
            pthread_cancel(tid);

            return;
        }
        
    }

    len = *(int *)msg;
    size = 0;
    memset(msg, 0, sizeof(int));

    while (1)
    {
        size += recv(g_sockfd, msg + size, len - size, 0);
        if (size >= len)
            break;
    }

    printf("[RECV] len: %d ; msg: %s\n", len, msg);
}


// 开始播放
void socket_start_play()
{
    struct json_object *obj = json_object_new_object();
    json_object_object_add(obj, "cmd", json_object_new_string("app_start_reply"));
    // 开始播放
    player_start_play();
    // 判断结果
    char result[128] = {0};
    FILE *fp = popen("pgrep mplayer", "r");
    fgets(result, 128, fp);

    if (strlen(result) == 0)
    {
        json_object_object_add(obj, "result", json_object_new_string("failure"));
    }
    else
    {
        json_object_object_add(obj, "result", json_object_new_string("success"));
    } 
    // 返回结果
    socket_send_data(obj);

    json_object_put(obj);
}


// 停止播放
void socket_stop_play()
{
    struct json_object *obj = json_object_new_object();
    json_object_object_add(obj, "cmd", json_object_new_string("app_stop_reply"));
    // 开始播放
    player_stop_play();
    // 判断结果
    char result[128] = {0};
    FILE *fp = popen("pgrep mplayer", "r");
    fgets(result, 128, fp);

    if (strlen(result) == 0)
    {   
        json_object_object_add(obj, "result", json_object_new_string("success"));
    }
    else
    {
        json_object_object_add(obj, "result", json_object_new_string("failure"));
    } 
    // 返回结果
    socket_send_data(obj);

    json_object_put(obj);
}


// 暂停播放
void socket_suspend_play()
{
    struct json_object *obj = json_object_new_object();
    json_object_object_add(obj, "cmd", json_object_new_string("app_suspend_reply"));
    // 暂停播放
    player_suspend_play();
    // 发送结果
    json_object_object_add(obj, "result", json_object_new_string("success"));
    // 返回结果
    socket_send_data(obj);

    json_object_put(obj);
}


// 继续播放
void socket_continue_play()
{
    struct json_object *obj = json_object_new_object();
    json_object_object_add(obj, "cmd", json_object_new_string("app_continue_reply"));
    // 继续播放
    player_continue_play();
    // 发送结果
    json_object_object_add(obj, "result", json_object_new_string("success"));
    // 返回结果
    socket_send_data(obj);

    json_object_put(obj);
}


// 下一首
void socket_next_play()
{
    struct json_object *obj = json_object_new_object();
    json_object_object_add(obj, "cmd", json_object_new_string("app_next_reply"));

    // 读取共享内存，获取当前歌曲
    Shm old_info, new_info;
    parent_get_shm(&old_info);
    // 切歌
    player_next_play();
    // 再次获取歌曲
    parent_get_shm(&new_info);
    // 回复信息
    if (strcmp(old_info.cur_music, new_info.cur_music))
    {
        // 歌曲不一样，判定为切歌成功
        json_object_object_add(obj, "result", json_object_new_string("success"));
        json_object_object_add(obj, "music", json_object_new_string(new_info.cur_music));
    } 
    else
    {
        // 歌曲一样，判定为切歌失败
        json_object_object_add(obj, "result", json_object_new_string("failure"));
    }

    // 返回结果
    socket_send_data(obj);

    json_object_put(obj);
}


// 上一首
void socket_prior_play()
{
    struct json_object *obj = json_object_new_object();
    json_object_object_add(obj, "cmd", json_object_new_string("app_prior_reply"));

    // 读取共享内存，获取当前歌曲
    Shm old_info, new_info;
    parent_get_shm(&old_info);
    // 切歌
    player_prior_play();
    // 再次获取歌曲
    parent_get_shm(&new_info);
    // 回复信息
    if (strcmp(old_info.cur_music, new_info.cur_music))
    {
        // 歌曲不一样，判定为切歌成功
        json_object_object_add(obj, "result", json_object_new_string("success"));
        json_object_object_add(obj, "music", json_object_new_string(new_info.cur_music));
    } 
    else
    {
        // 歌曲一样，判定为切歌失败
        json_object_object_add(obj, "result", json_object_new_string("failure"));
    }

    // 返回结果
    socket_send_data(obj);

    json_object_put(obj);
}


// 增加音量
void socket_volumn_up()
{
    struct json_object *obj = json_object_new_object();
    json_object_object_add(obj, "cmd", json_object_new_string("app_voice_up_reply"));
    // 获取当前音量
    int old;
    device_get_volume(&old);
    if (old == 100)
    {
        // 已经是最大音量
        json_object_object_add(obj, "result", json_object_new_string("success"));
        json_object_object_add(obj, "voice", json_object_new_int(100));
        socket_send_data(obj);
        json_object_put(obj);
        return;
    }
    // 调整音量
    player_volumn_up();
    // 再次获取
    int new;
    device_get_volume(&new);
    // 比较并返回结果
    if (new <= old)
    {
        // 调节失败
        json_object_object_add(obj, "result", json_object_new_string("failure"));
    }
    else
    {
        // 调节成功
        json_object_object_add(obj, "result", json_object_new_string("success"));
        json_object_object_add(obj, "voice", json_object_new_int(new));
    }

    socket_send_data(obj);
    json_object_put(obj);
}


// 降低音量
void socket_volumn_down()
{
    struct json_object *obj = json_object_new_object();
    json_object_object_add(obj, "cmd", json_object_new_string("app_voice_down_reply"));
    // 获取当前音量
    int old;
    device_get_volume(&old);
    if (old == 0)
    {
        // 已经是最小音量
        json_object_object_add(obj, "result", json_object_new_string("success"));
        json_object_object_add(obj, "voice", json_object_new_int(0));
        socket_send_data(obj);
        json_object_put(obj);
        return;
    }
    // 调整音量
    player_volumn_down();
    // 再次获取
    int new;
    device_get_volume(&new);
    // 比较并返回结果
    if (new >= old)
    {
        // 调节失败
        json_object_object_add(obj, "result", json_object_new_string("failure"));
    }
    else
    {
        // 调节成功
        json_object_object_add(obj, "result", json_object_new_string("success"));
        json_object_object_add(obj, "voice", json_object_new_int(new));
    }

    socket_send_data(obj);
    json_object_put(obj);
}
