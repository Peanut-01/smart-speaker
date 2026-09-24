#ifndef PLAYER_H
#define PLAYER_H

#include <time.h>
#include <list>
#include <iostream>
#include <event.h>
#include <jsoncpp/json/json.h>


class Server;  // 前向声明Server类


struct PlayerInfo
{
    std::string deviceid;
    std::string appid;
    std::string cur_music;
    int volume;
    int mode;
    time_t d_time;  // 记录音箱上报的时间
    time_t a_time;  // 记录APP上报的时间

    struct bufferevent* d_bev;  // 对应音箱事件
    struct bufferevent* a_bev;  // 对应app事件
};

class Player
{
private:
    std::list<PlayerInfo> *info;  // 音箱信息链表

public:
    Player();
    ~Player();

    void player_update_list(struct bufferevent* bev, Json::Value &value, Server* s);
    void player_app_update_list(struct bufferevent* bev, Json::Value &value);
    void player_traverse_list();
};

#endif // PLAYER_H
