#if !defined(SERVER_H)
#define SERVER_H

#include <event.h>
#include <event2/listener.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <list>
#include <time.h>
#include <stdio.h>
#include <jsoncpp/json/json.h>
#include <sys/types.h>
#include <dirent.h>
#include "database.h"
#include "player.h"


#define IP "172.28.208.58"
#define PORT 8000

#define SEQUENCE 1
#define CIRCLE 2


class Server 
{
private: 
    struct event_base* m_base;       // 事件集合
    Database* m_database;            // 数据库对象
    Player* m_player;                // 音箱对象
public:
    Server();
    ~Server();
    struct event_base* server_get_base();

    void server_read_data(struct bufferevent* bev, char* msg);
    void listen(const char* ip, int port);
    void server_get_music(struct bufferevent* bev, std::string singer);
    void server_send_data(struct bufferevent* bev, Json::Value &value);
    void server_player_handler(struct bufferevent* bev, Json::Value &value);
    void server_start_timer();

    static void listener_cb(struct evconnlistener *, evutil_socket_t , struct sockaddr * , int socklen, void* );
    static void read_cb(struct bufferevent* bev, void* arg);
    static void event_cb(struct bufferevent* bev, short events, void* arg);
    static void timeout_cb(evutil_socket_t fd, short event, void *arg);
};


#endif // SERVER_H
