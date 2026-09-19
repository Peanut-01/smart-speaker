#include <iostream>
#include "server.h"


Server::Server() {
    m_base = event_base_new();
}


// Server::~Server() {
//     if (m_base) {
//         event_base_free(m_base);
//     }
// }


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
    std::cout << "New connection accepted";
    std::cout << " from " << inet_ntoa(client_addr->sin_addr) << ":" << ntohs(client_addr->sin_port) << std::endl;
    // 在这里可以处理新的连接，例如创建一个新的事件来处理客户端请求
}
