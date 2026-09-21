#if !defined(SERVER_H)
#define SERVER_H

#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <list>
#include <time.h>
#include <jsoncpp/json/json.h>
#include "database.h"

#define IP "172.28.208.58"
#define PORT 8000

#define SEQUENCE 1
#define CIRCLE 2

struct PlayerInfo
{
    char deviceid[16];
    char appid[16];
    char music[128];
    int volume;
    int mode;
    time_t time;

    struct bufferevent* d_bev;  // 对应音箱事件
    struct bufferevent* a_bev;  // 对应app事件
};

class Server 
{
private: 
    struct event_base* m_base;       // 事件集合
    Database* m_database;            // 数据库对象
    std::list<PlayerInfo> *m_info;  // 音箱信息链表
public:
    Server();
    ~Server();
    struct event_base* server_get_base();

    void server_read_data(struct bufferevent* bev, char* msg);
    void listen(const char* ip, int port);
    static void listener_cb(struct evconnlistener *, evutil_socket_t , struct sockaddr * , int socklen, void* );
    static void read_cb(struct bufferevent* bev, void* arg);
    static void event_cb(struct bufferevent* bev, short events, void* arg);
};


#endif // SERVER_H
