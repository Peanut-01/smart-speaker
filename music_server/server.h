#if !defined(SERVER_H)
#define SERVER_H

#include <event2/event.h>
#include <event2/listener.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define IP "172.28.208.58"
#define PORT 8000

class Server 
{
private: 
    struct event_base* m_base;

public:
    Server();
    //~Server();

    void listen(const char* ip, int port);
    static void listener_cb(struct evconnlistener *, evutil_socket_t , struct sockaddr * , int socklen, void* );

};


#endif // SERVER_H
