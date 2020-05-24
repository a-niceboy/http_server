#ifndef _DATA_CENTER_H_
#define _DATA_CENTER_H_
#include "HttpHeader.h"
#include "Http.h"
#include "SqlInterface.h"
#include "typedefine.h"

class DataCenter 
{
public:
	DataCenter();
	~DataCenter();

	void do_sql(string& out_data, const vector<uint8_t>& bodyCvec);

private:
	SqlInterface *m_sql;
};

#endif
