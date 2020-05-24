#ifndef _HTTP_HEADER_H_
#define _HTTP_HEADER_H_

#include "typedefine.h"

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 87380
#endif

class HttpHeaderItem
{
public:
	HttpHeaderItem();
	//HttpHeaderItem(const HttpHeaderItem &) = delete;
	//HttpHeaderItem(HttpHeaderItem &&) noexcept = default;
	
	HttpHeaderItem(const string &sKey, const string &sValue);
	HttpHeaderItem(string &&sKey, string &&sValue);
	void load(const string &sKey, const string &sValue);
	void emplace(string &&sKey, string &&sValue);
	void clear();
	void show(ostream &os = cout) const;
	
public:
	string m_sKey;
	string m_sValue;	
};

class HttpHeader
{
public:
	HttpHeader() {}
	//HttpHeader(const HttpHeader &) = delete;
	//HttpHeader(HttpHeader &&) noexcept = default;
	
	void clear();
	void show(ostream &os = cout) const;
	string operator [](const string &sKey) const;
    bool empty() const;
    
    void insert(const string &sKey, const string &sValue);
    void emplace(string &&sKey, string &&sValue);
	void getCookie(string &sCookie) const;
	
public:
	vector<HttpHeaderItem> m_HttpheaderVec;
};


#endif
