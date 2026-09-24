#include "player.h"
#include "server.h"

Player::Player() 
{
    // 初始化链表对象
    info = new std::list<PlayerInfo>;
}

Player::~Player() 
{
    if (info)
        delete info;
}


void Player::player_update_list(struct bufferevent* bev, Json::Value &value, Server* s)
{
    auto it = info->begin();
    // 遍历链表，如果设备存在，则更新链表，并转发给 APP
    for (; it != info->end(); ++it) 
    {
        if (it->deviceid == value["deviceid"].asString()) 
        {
            std::cout << "Device " << it->deviceid << " exists, updating info." << std::endl;
            // 更新链表中的设备信息
            it->cur_music = value["cur_music"].asString();
            it->volume = value["volume"].asInt();
            it->mode = value["mode"].asInt();
            it->d_time = time(NULL);

            // 设备在线，则转发给 APP
            if (it->a_bev) 
            {
                std::cout << "APP在线, 数据转发给APP" << std::endl;
                s->server_send_data(it->a_bev, value);
            }
            return;
        }
    }

    // 如果设备不存在，则新建节点
    PlayerInfo new_p;
    new_p.deviceid = value["deviceid"].asString();
    new_p.cur_music = value["cur_music"].asString();
    new_p.volume = value["volume"].asInt();
    new_p.mode = value["mode"].asInt();
    new_p.d_time = time(NULL);
    new_p.d_bev = bev;
    new_p.a_bev = NULL;

    info->push_back(new_p);

    std::cout << "New device " << new_p.deviceid << " added to the list." << std::endl;
}


void Player::player_app_update_list(struct bufferevent* bev, Json::Value &value)
{
    for (auto it = info->begin(); it != info->end(); ++it) 
    {
        if (it->deviceid == value["deviceid"].asString()) 
        {
            it->a_time = time(NULL);
            it->appid = value["appid"].asString();
            it->a_bev = bev;
            std::cout << "APP info updated for device " << it->deviceid << std::endl;
            return;
        }
    }
}


// 遍历链表，删除离线的音箱和APP
void Player::player_traverse_list()
{
    std::cout << "定时器事件：遍历链表" << std::endl;
    for (auto it = info->begin(); it != info->end(); ++it) 
    {
        if (time(NULL) - it->d_time > 6)    // 超过3次没有收到数据，音箱离线，删除该节点
        {
            info->erase(it);
        }

        if (it->a_bev && time(NULL) - it->a_time > 6)    // 超过3次没有收到数据，APP离线，删除该节点
        {
            it->a_bev = NULL;
        }
    }
}
