#ifndef  __SQL_INTERFACE_H__
#define  __SQL_INTERFACE_H__
#include <mysql/mysql.h>
#include "typedefine.h"

class SqlInterface
{
public:
    SqlInterface();
    ~SqlInterface();
    
    bool init(const string& domain = "ip", const string& user = "user", const string& password = "password", const string& database = "basedata_name", const int& port = 80);

	bool insert_data();
	bool delete_data();
	bool update_data();
	bool selete_data(int& id);
private:
	//连接
	MYSQL* conn;
	//返回
	MYSQL_RES* result;
	//信息
	MYSQL_ROW row;
};

#endif
