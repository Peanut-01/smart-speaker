#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include "socket.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>
#include <pthread.h>
#include <json/json.h>


// 发送数据到客户端
void server_send_data(int fd, struct json_object *obj)
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

    printf("发送 %d 字节\n", len);

    if (send(fd, buf, len + 4, 0) == -1) 
    {
        perror("send server");
        return;
    }    
}


// 接收客户端的请求数据
void *recv_client(void* arg)
{
    int fd = *(int *)arg;
    char buf[1024] = {0};
    int len = 0;
    size_t size = 0;

    while (1)
    {   
        // 接收有效数据长度
        while (1)
        {
            size += recv(fd, buf + size, sizeof(int) - size, 0);
            if (size >= sizeof(int))
                break;
        }

        size = 0;
        len = *(int *)buf;
        memset(buf, 0, sizeof(buf));
        printf("收到 %d 字节: ", len);

        // 接收有效数据
        while (1)
        {
            size += recv(fd, buf + size, len - size, 0);
            if (size >= len)
                break;
        }
        
        printf("%s\n", buf);

        // 把字符串转化成json对象
        struct json_object *obj = json_tokener_parse(buf);
        struct json_object *val = json_object_object_get(obj, "cmd");
        if (strcmp("get_music_list", json_object_get_string(val)) == 0)
        {
            // 返回音乐数据
            struct json_object *snd_obj = json_object_new_object();
            json_object_object_add(snd_obj, "cmd", json_object_new_string("reply_music"));

            struct json_object *array = json_object_new_array();
            json_object_array_add(array, json_object_new_string("其他/以后的以后.mp3"));
            json_object_array_add(array, json_object_new_string("其他/倾国倾城.mp3"));
            json_object_array_add(array, json_object_new_string("其他/童话.mp3"));
            json_object_array_add(array, json_object_new_string("其他/那些年.mp3"));
            json_object_array_add(array, json_object_new_string("其他/一直想着他.mp3"));

            json_object_object_add(snd_obj, "music", array);

            server_send_data(fd, snd_obj);
        }


        memset(buf, 0, sizeof(buf));
        size = 0;
    }
}


int main()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == sockfd)
    {
        perror("socket");
        return -1;
    }

    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 创建服务端的结构体并填写信息
    struct sockaddr_in server_info;
    memset(&server_info, 0, sizeof(server_info));

    server_info.sin_family = AF_INET;
    server_info.sin_port = htons(PORT);
    server_info.sin_addr.s_addr = inet_addr(IP);
    socklen_t server_len = sizeof(server_info);

    if (bind(sockfd, (struct sockaddr *)&server_info, server_len) == -1)
    {
        perror("bind");
        return -1;
    }

    if (listen(sockfd, 10) == -1)
    {
        perror("listen");
        return -1;
    }

    struct sockaddr_in client_info;
    socklen_t client_len = sizeof(client_info);

    int fd = accept(sockfd, (struct sockaddr *)&client_info, &client_len);
    
    if (fd == -1)
    {
        perror("accept");
    }

    printf("接收客户端的连接 %d\n", fd);

    pthread_t tid;
    pthread_create(&tid, NULL, recv_client, &fd);



    while (1)
    {
        /* code */
    }

    close(fd);
    close(sockfd);

    return 0;
}
