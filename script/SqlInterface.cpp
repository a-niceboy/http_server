#include "SqlInterface.h"

inline void show_error(MYSQL* conn, const string& sql)
{
	printf("Error(%d) [%s] \"%s\"", mysql_errno(conn), mysql_sqlstate(conn), mysql_error(conn));
	printf("\nsql:\n%s\n", sql.c_str());
	mysql_close(conn);
}

SqlInterface::SqlInterface()
{

}

SqlInterface::~SqlInterface()
{
	mysql_close(conn);
	mysql_library_end();
}

bool SqlInterface::init(const string& domain, const string& user, const string& password, const string& database, const int& port)
{
	if (mysql_library_init(0, NULL, NULL))
	{
		printf("could not initialize conn library\n");
		system("pause");
		exit(1);
	}

	conn = mysql_init(NULL);

	if (!mysql_real_connect(conn, domain.c_str(), user.c_str(), password.c_str(),
		database.c_str(), port, "/var/lib/mysql/mysql.sock", 0))
	{
		show_error(conn, "connect");
		return false;
	}

	if (!mysql_set_character_set(conn, "utf8"))
	{
		printf("Character for the connection : %s\n",
			mysql_character_set_name(conn));
	}

	return true;
}

bool SqlInterface::insert_data()
{
	string insert_sql = "insert into from xxx (id) value(1)";
	if (mysql_query(conn, insert_sql.c_str()))
	{
		show_error(conn, insert_sql);
		return false;
	}
	return true;
}
bool SqlInterface::delete_data()
{
	string delete_sql = "delete from xxx where id = 1";
	if (mysql_query(conn, delete_sql.c_str()))
	{
		show_error(conn, delete_sql);
		return false;
	}
	return true;
}
bool SqlInterface::update_data()
{
	string update_sql = "update set xxx id = 1";
	if (mysql_query(conn, update_sql.c_str()))
	{
		show_error(conn, update_sql);
		return false;
	}
	return true;
}
bool SqlInterface::selete_data(int& id)
{
	string selete_sql = "select max(id) from xxx";
	if (mysql_query(conn, selete_sql.c_str()))
	{
		show_error(conn, selete_sql);
		return false;
	}
	else
	{
		result = mysql_store_result(conn);
		if (NULL == result)
		{
			show_error(conn, selete_sql);
			return false;
		}
		else
		{
			int num_rows = mysql_num_rows(result);
			if (num_rows != 1)
			{
				mysql_free_result(result);
				return false;
			}
			else
			{
				row = mysql_fetch_row(result);
				if (row < 0)
				{
					mysql_free_result(result);
					return false;
				}

				if (row[0] != NULL) { id = lexical_cast<int>(row[0]); }
			}
			mysql_free_result(result);
		}
	}
	return true;
}
