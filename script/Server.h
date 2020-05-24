#ifndef _SERVER_H_
#define _SERVER_H_

#include "typedefine.h"
#include "TimeManager.h"
#include "KEncode.h"
#include "SqlInterface.h"
#include "HttpHeader.h"
#include "DataCenter.h"

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 87380
#endif

enum HTTP_TYPE
{
    GET,
    POST
};

class ServerConfig
{
public:
	ServerConfig()
	{
		m_port = 0;
		m_listenNum = 0;
		m_threadPoolSize = 0;
	}
	
	ServerConfig(const short int port, const int listenNum, const int threadPoolSize)
	{
		load(port, listenNum, threadPoolSize);
	}
	
	void load(const short int port, const int listenNum, const int threadPoolSize)
	{
		m_port = port;
		m_listenNum = listenNum;
		m_threadPoolSize = threadPoolSize;
	}
	
	void clear()
	{
		m_port = 0;
		m_listenNum = 0;
		m_threadPoolSize = 0;	
	}
	
	void show(ostream &os = cout) const
	{
		os << "====ServerConfig====" << endl;
		os << "m_port = " << m_port << endl;
		os << "m_listenNum = " << m_listenNum << endl;
		os << "m_threadPoolSize = " << m_threadPoolSize << endl;
	}
	
public:	
	short int m_port;
	int m_listenNum;
	int m_threadPoolSize;
};

class ServerPoolWorker
{
public:
	virtual ~ServerPoolWorker()
	{
	}
	
    virtual void proccess(int newfd) = 0;
    virtual void onThreadCreate(pthread_t tid)
    {
    	
    }
};

class ServerThreadPool
{
public:
	ServerThreadPool();
	~ServerThreadPool();
	
	void init(int listenfd, const int max_thread_num);
	int destroy();
	void *thread_routine();	
	
	void setCallBack(ServerPoolWorker *server);
	
public: 
	int m_listenFd;
    pthread_mutex_t queue_lock;
    vector<pthread_t> threadidVec;
    bool shutdown;
    
    ServerPoolWorker *m_server;
};

class HttpServer : public ServerPoolWorker
{
public:
	HttpServer(const ServerConfig &config);
	~HttpServer();

	void showSockBuffSize(int sockfd, const int optname);

	bool start();
	int doJob();
	
    void getVPath(string &sVPath, const string &sContent, const HTTP_TYPE type);
    
    pair<string, string> divideArg(const string &url, const string &sDelem);
    void parserUrl(URLContainer &urlArgMap, const string &sVPath);
    
    void do_get(int newfd, const string &sHead);
    void do_post(int newfd, const string &sHead, vector<uint8_t> &BodyCvec);
    
	int sendHttpData(int newfd, const vector<uint8_t> &cvec, const HttpHeader *header, bool needkencode = false);
	int sendHttpData(int newfd, const string &sContent, const HttpHeader *header, bool needkencode = false);
    
    int getContentLength(const string &s);
    pair<int, int> getRang(const string &sRecv);
	
	void parseRequestHeader(const string &sHead, HttpHeader &mapHeader) const;
	void makeRespHeader(string &s, size_t contentLength, const HttpHeader *header) const;
	
private:
	HttpServer();
	
    void proccess(int newfd);
    void onThreadCreate(pthread_t tid);
	
protected:
	int m_sockfd;
	ServerThreadPool m_threadPool;
	const ServerConfig m_config;
    
	DataCenter m_data_center;
};

#endif
