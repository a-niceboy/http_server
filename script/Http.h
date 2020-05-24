/*
 *  Http.h
 *  TGBaidu
 *
 *  Created by xiapeng on 11-4-26.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _HTTP_CLIENT_H_
#define _HTTP_CLIENT_H_

#include "typedefine.h"
#include "HttpHeader.h"

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 87380
#endif

class URL
{
public:
	URL()
	{
	}
	
	URL(const string &sUrl)
	{
        init(sUrl);
	}
	
	void show(ostream &os = cout) const
	{
	    os << "==============" << endl;
		os << "Host:" << m_sHost << endl;
		os << "Path:" << m_sPath << endl;
		os << "Port:" << m_Port << endl;	
	}
	
	void clear()
	{
		m_sHost = "";
		m_sPath = "";
		m_Port = 0;	
	}
	
	void init(const string &sUrl)
	{
	    const string sHttpWord("http://");
	    const string sHttpWord1("https://");
	    
	    bool isHttps = false;
	    string::size_type pos1 = sUrl.find(sHttpWord);
	    
	    string sData;
	    if(pos1 == string::npos)
	    {
	    	pos1 = sUrl.find(sHttpWord1);
	        if(pos1 == string::npos)
	        {
	        	sData = sUrl;
	        }
	        else
	        {
	        	sData = sUrl.substr(pos1 + sHttpWord1.size());
	        	isHttps = true;
	        }
	    }
	    else
	    {
	        sData = sUrl.substr(pos1 + sHttpWord.size());
	    }
	    
	    pos1 = 0u;
	    string sData1;
	    string::size_type pos2 = sData.find("/");
	    if(pos2 == string::npos)
	    {
	        sData1 = sData;
	    }
	    else
	    {
			sData1 = sData.substr(0, pos2);
			m_sPath = sData.substr(pos2);
	    }
	    
	    string::size_type pos3 = sData1.find(":");
	    if(pos3 == string::npos)
	    {
            m_sHost = sData1;
            
            if(isHttps == false)
            {
		        m_Port = 80;
		    }
		    else
		    {
		        m_Port = 443;
		    }
	    }
	    else
	    {
			m_sHost = sData1.substr(0, pos3);
			m_Port = atoi(sData1.substr(pos3 + 1).c_str());
	    }
	}
	
	const string &host() const
	{
	    return m_sHost;
	}
	
	const string &path() const
	{
	    return m_sPath;
	}
	
	const short port() const
	{
	    return m_Port;
	}
	
	const string wholeUrl() const
	{
		string sWholeUrl = "http://" + m_sHost + ":" + lexical_cast<string>(m_Port) + "/" + m_sPath;
	    return sWholeUrl;
	}
	
private:
	string m_sHost;
	string m_sPath;
	short m_Port;
};

class Post11FormItem
{
public:
	void clear();
	void show(ostream &os = cout) const;
    
public:
    string m_sName;
    string m_sAction;
    string m_sFileName;
    string m_sContentType;
    ByteArray m_cvec;
    string m_sBoundary;
};

class ProxyBag
{
public:
	enum ProxyBagType
	{
		EHTTPS,
		ESOCK5
	};
	
    ProxyBag()
	{
	    m_port = 0;
	    m_ttl = 0;
	    m_reused = 0;
	}
	
	bool set(const string &sIpPort, ProxyBagType type)
	{
	    string::size_type pos = sIpPort.find(":");
	    if(pos == string::npos)
	    {
            return false;
	    }
	    
    	m_sDomain = sIpPort.substr(0, pos);
    	m_port = atoi(sIpPort.substr(pos + 1).c_str());
    	
    	m_type = type;
    	
    	return true;
	}

    ProxyBag(const string &sDomain, const unsigned short port, ProxyBagType type)
		: m_sDomain(sDomain)
	{
	    m_port = port;
	    m_ttl = 0;
	    m_reused = 0;
	    m_type = type;
	}

	void clear()
	{
	    m_sDomain.clear();
        m_port = 0;
        m_ttl = 0;
        m_reused = 0;
	}

	void show(ostream &os = cout) const
	{
	    os << "========ProxyBag========" << endl;
		os << "m_sDomain = " << m_sDomain << endl;
		os << "m_port = " << m_port << endl;
		os << "m_ttl = " << m_ttl << endl;
		os << "m_reused = " << m_reused << endl;
		os << "m_type = " << m_type << endl;
	}

	bool empty() const
	{
	    return m_sDomain.empty();
	}

    string toString() const
	{
		return m_sDomain + ":" + lexical_cast<string>(m_port);
	}
	
public:
	string m_sDomain;
	unsigned short m_port;
	int m_ttl;
	int m_reused;
	
	ProxyBagType m_type;
};

class HttpClient
{
    friend class ProxyBag;

public:
	HttpClient();
	HttpClient(const ProxyBag &bag);
	~HttpClient();
	
	void setBag(const ProxyBag &bag);
	
	int Get2(const URL &url, vector<uint8_t> &cvec, const HttpHeader *pInputHeader = NULL, HttpHeader *pheader = NULL, long timeout = 20);
	
	int Post(const URL &url, const vector<uint8_t> &data, vector<uint8_t> &cvec, const HttpHeader *pInputHeader = NULL, HttpHeader *pheader = NULL, long timeout = 20);
    int Post11(const URL &url, const vector<Post11FormItem> &data, vector<uint8_t> &cvec, const HttpHeader *pInputHeader, HttpHeader *pheader = NULL, long timeout = 20);
    
	int gzcompress(const unsigned char *data, unsigned long ndata, unsigned char *zdata, unsigned long *nzdata) const;
    int ungzipxp(const uint8_t *source, const int len, vector<uint8_t> &desvec) const;
    
protected:
	bool Connect(const char *domain, const int port, string &sIp);
	bool doConnectHttps(const string &sServerName, uint16_t port1);
	bool doConnectSock5(const string &sServerName, uint16_t port1);
	
	int getContentLength(const string &s) const;
	int recvHttpData(string &sRecv, long timeout);
	int recvHttpData(vector<uint8_t> &cvec, long timeout);
	int recvHttpData(vector<uint8_t> &cvec, const string &sFileName, long timeout);
	
	int recvHttpData2(vector<uint8_t> &cvec, long timeout);
	int recvAChunck(vector<uint8_t> &chunckData, long timeout);
	
	HttpHeaderItem divideArg(const string &sLine, const string &sDelem) const;
	bool makeRespHead(const string &sHead, HttpHeader &header) const;
    void makeGetRequestHead(string &s0, const URL &url, const HttpHeader *phead, const int beg_pos = -1, const int end_pos = -1) const;
	void makePostRequestHead(string &s0, const URL &url, const HttpHeader *phead, const vector<uint8_t> &data) const;

protected:
	int m_sockfd;
	ProxyBag m_bag;
};

string domainToIp(const string &sDomain);

#endif
