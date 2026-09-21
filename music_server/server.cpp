#include <iostream>
#include "server.h"


Server::Server() {
    // 初始化事件集合
    m_base = event_base_new();
    // 初始化数据库对象
    m_database = new Database();
    if (!m_database->database_init_table()) 
    {
        std::cout << "Failed to initialize database table" << std::endl;
        exit(1);
    }

    // 初始化链表对象
    m_info = new std::list<PlayerInfo>;
}


Server::~Server() {
    if (m_database) {
        delete m_database;
    }
    if (m_info) {
        delete m_info;
    }
}


void Server::listen(const char* ip, int port) {
    struct sockaddr_in server_info;
    int len = sizeof(server_info);
    memset(&server_info, 0, len);
    server_info.sin_family = AF_INET;
    server_info.sin_addr.s_addr = inet_addr(ip);
    server_info.sin_port = htons(port);
    // 创建监听对象
    struct evconnlistener* listener = evconnlistener_new_bind(m_base, listener_cb, this, 
        LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, 5, (struct sockaddr*)&server_info, len);
    if (NULL == listener) 
    {
        std::cout << "bind error" << std::endl;
        return;
    }

    // 监听集合，一般是死循环
    event_base_dispatch(m_base);

    // 释放对象
    evconnlistener_free(listener);
    event_base_free(m_base);
}


// 一旦有客户端发起连接请求，就会触发该函数
void Server::listener_cb(struct evconnlistener* listener, evutil_socket_t fd, 
    struct sockaddr* addr, int socklen, void* arg) {
    
    struct sockaddr_in* client_addr = (struct sockaddr_in*)addr;
    // struct event_base* base = (struct event_base*)arg;
    Server *s = (Server*)arg;
    struct event_base* base = s->server_get_base();

    std::cout << "New connection accepted";
    std::cout << " from " << inet_ntoa(client_addr->sin_addr) << ":" << ntohs(client_addr->sin_port) << std::endl;

    // 在这里可以处理新的连接，例如创建一个新的事件来处理客户端请求
    struct bufferevent* bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    if (NULL == bev) 
    {
        std::cout << "bufferevent_socket_new error" << std::endl;
        return;
    }

    bufferevent_setcb(bev, read_cb, NULL, event_cb, s);
    bufferevent_enable(bev, EV_READ);

}


// 有客户端发数据，会触发该函数
void Server::read_cb(struct bufferevent* bev, void* ctx) {
    Server *s = (Server*)ctx;
    char buf[1024] = {0};
    s->server_read_data(bev, buf);

    // 解析json
    Json::Reader reader;    // 创建一个Json解析器
    Json::Value value;      // 存放解析后的结果
    if (!reader.parse(buf, value)) 
    {
        std::cout << "Failed to parse JSON: " << std::endl;
        return;
    }

    std::cout << value << std::endl;
}


void Server::server_read_data(struct bufferevent* bev, char* msg)
{
    char buf[8] = {0};
    size_t size = 0;
    while (1)
    {
        size += bufferevent_read(bev, buf + size, 4 - size);
        if (size >= 4)
            break;
    }

    int len = *(int *)buf;
    size = 0;
    while (true)
    {
        size += bufferevent_read(bev, msg + size, len - size);
        if (size >= len)
            break;
    }

    std::cout << "Received " << len << " bytes: " << msg << std::endl;
}


void Server::event_cb(struct bufferevent* bev, short events, void* arg) {
    if (events & BEV_EVENT_EOF) {
        std::cout << "Connection closed" << std::endl;
    } else if (events & BEV_EVENT_ERROR) {
        std::cout << "Error on connection" << std::endl;
    }
    bufferevent_free(bev);
}


// 获取事件集合
struct event_base* Server::server_get_base() {
    return m_base;
}
