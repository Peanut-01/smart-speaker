#include "database.h"


Database::Database() 
{

}

Database::~Database() 
{

}


bool Database::database_connect() 
{
    // 初始化 MySQL 句柄
    mysql = mysql_init(NULL);
    if (mysql == NULL) 
    {
        std::cout << "mysql_init() failed" << std::endl;
        return false;
    }

    // 连接到数据库
    if (mysql_real_connect(mysql, "localhost", "root", "root", "musicplayer", 0, NULL, 0) == NULL) 
    {
        std::cout << "[DATABASE CONNECT ERROR]";
        std::cout << mysql_error(mysql) << std::endl;
        mysql_close(mysql);
        return false;
    }

    // if (mysql_query(mysql, "SET NAMES utf8") != 0) 
    // {
    //     std::cout << "[DATABASE CONNECT ERROR]";
    //     std::cout << mysql_error(mysql) << std::endl;
    //     mysql_close(mysql);
    //     return false;
    // }

    return true;
}


void Database::database_disconnect() 
{
    if (mysql != NULL) 
    {
        mysql_close(mysql);
        mysql = NULL;
    }
}


bool Database::database_init_table() 
{
    if (!this->database_connect())
        return false;

    // 创建数据表的 SQL 语句
    const char* create_table_sql = "CREATE TABLE IF NOT EXISTS account ("
                                   "appid char(11) NOT NULL,"
                                   "password varchar(16) NOT NULL,"
                                   "deviceid varchar(8) NOT NULL"
                                   ")charset=utf8;";

    // 执行 SQL 语句
    if (mysql_query(mysql, create_table_sql) != 0) 
    {
        std::cout << "[DATABASE INIT TABLE ERROR]";
        std::cout << mysql_error(mysql) << std::endl;
        return false;
    }

    this->database_disconnect();

    return true;
}
