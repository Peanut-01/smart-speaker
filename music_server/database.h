#if !defined(DATABASE_H)
#define DATABASE_H

#include <iostream>
#include <mysql/mysql.h>

class Database
{
private:
    MYSQL* mysql;   // 数据库句柄
public:
    Database();
    ~Database();
    bool database_connect();
    void database_disconnect();
    bool database_init_table();
};

#endif // DATABASE_H
