#include "Server.h"

/*----------------------------------------------------------------------------*/
ServerThreadPool::ServerThreadPool()
{
    shutdown = false;
    m_listenFd = 0;
}

ServerThreadPool::~ServerThreadPool()
{
	destroy();
}

union ConvertProc
{
    void *(*ThreadProc)(void *);
    void *(ServerThreadPool::*MemberProc)();
};

void ServerThreadPool::init(int listenfd, const int max_thread_num)
{
	pthread_mutex_init(&queue_lock, NULL);
	
	m_listenFd = listenfd;
	
    shutdown = false;
    threadidVec.resize(max_thread_num);
    
    ConvertProc cp;
    cp.MemberProc = &ServerThreadPool::thread_routine;
      
    for(int i = 0; i < max_thread_num; i++) 
    {
        pthread_create(&threadidVec[i], NULL, cp.ThreadProc, this);
    } 
}

int ServerThreadPool::destroy() 
{ 
    if(shutdown == true)
    {
        return -1;
    }
    
    shutdown = true;

    for(size_t i = 0; i < threadidVec.size(); i++)
    { 
        pthread_join(threadidVec[i], NULL);
    }
    
    pthread_mutex_destroy(&queue_lock); 

    return 0; 
} 

void *ServerThreadPool::thread_routine()
{ 
    printf("starting thread 0x%lx\n", pthread_self()); 
    while(true) 
    { 
        pthread_mutex_lock(&queue_lock);
        
        //printf("thread 0x%lx is waiting\n", pthread_self());
        sockaddr_in their_addr;
        socklen_t sin_size = sizeof(sockaddr_in);
        
        int new_fd = accept(m_listenFd, reinterpret_cast<sockaddr*>(&their_addr), &sin_size);
        if(new_fd == -1)
        {
            perror("accept error!");
            pthread_mutex_unlock(&queue_lock);
            continue;
        }
        
        pthread_mutex_unlock(&queue_lock);
        
        cout << "==== accept success" << endl;
        cout << "time: " << TimeManager(time(NULL)).toString("%Y-%m-%d %H:%M:%S") << endl;
        
        char addressBuffer[INET_ADDRSTRLEN] = "";
        inet_ntop(AF_INET, &their_addr.sin_addr, addressBuffer, INET_ADDRSTRLEN);
        cout << "client ip: " << addressBuffer << endl;

        if(shutdown == true)
        { 
            printf("thread 0x%lx will exit\n", pthread_self());
            pthread_exit(NULL); 
        }

        printf("thread 0x%lx is starting to work\n", pthread_self()); 
         
        if(m_server != NULL)
        {
        	m_server->proccess(new_fd);
        }
        
        close(new_fd);
    } 
    
    return NULL;
}

void ServerThreadPool::setCallBack(ServerPoolWorker *server)
{
	m_server = server;
}

/*-----------------------------------------------------------------------------*/
HttpServer::HttpServer(const ServerConfig &config)
	: m_sockfd(0), m_config(config)
{
	g_IM.pServer = this;
	m_threadPool.setCallBack(this);
}

HttpServer::~HttpServer()
{
	g_IM.pServer = NULL;
}

inline void HttpServer::showSockBuffSize(int sockfd, const int optname)
{
    int iRecvBuffSize = 0;
    socklen_t iIntSize = sizeof(int);
    getsockopt(m_sockfd, SOL_SOCKET, optname, &iRecvBuffSize, &iIntSize);
    cout << "iRecvBuffSize = " << iRecvBuffSize << endl; 	
}

bool HttpServer::start()
{
    sockaddr_in my_addr;
    
    if((m_sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("create socket error");
        perror("socket");
        return false;
    }
    
    socklen_t val = 1;
    if(setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0) 
    {
        cerr << "set SO_REUSEADDR error" << endl;
        close(m_sockfd);
        
        return false;
    }

    showSockBuffSize(m_sockfd, SO_RCVBUF);
    
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(m_config.m_port);
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(m_sockfd, (sockaddr*)&my_addr, sizeof(sockaddr)) == -1)
    {
        perror("bind socket error");
        close(m_sockfd);
        return false;
    }

    if(listen(m_sockfd, m_config.m_listenNum) == -1) 
    {
        perror("listen");
        close(m_sockfd);
        return false;
    }
   
    return true;
}

int HttpServer::doJob()
{
    if(start() == false)
    {
    	return -1;
    }
    cout << "server run" << endl;
    cout << "m_sockfd: " << m_sockfd << endl;

    m_threadPool.init(m_sockfd, m_config.m_threadPoolSize);
       
    while(true)
    {    	
        sleep(3000);	
    }
}

void HttpServer::proccess(int newfd)
{
    //printf("threadid is 0x%lx, working on socket %d\n", pthread_self(), newfd);
    
    vector<uint8_t> HeadCvec;
    vector<uint8_t> BodyCvec;
    char szBuff1[BUFFER_SIZE];
    	
    for(int i = 0; true; ++i)
    {
		pollfd pollfd;
		pollfd.fd = newfd;
		pollfd.events = POLLIN | POLLHUP | POLLPRI;

		int retval = poll(&pollfd, 1, 10 * 1000);
        if(retval < 0)
        {
        	cout << "poll error" << endl;
       	    return;
        }
        else if(retval == 0)
        {
        	cout << "no data in the 3 second" << endl;
            return;	
        }	
    
        memset(szBuff1, 0, BUFFER_SIZE);

        int iBytes = recv(newfd, szBuff1, BUFFER_SIZE, 0);
        if(iBytes < 0)
        {
            cerr << "Can't recv http head" << endl;
            return;
        }
        else if(iBytes == 0)
        {
            cerr << "peer closed" << endl;
            return;
        }
        
        copy(szBuff1, szBuff1 + iBytes, back_inserter(HeadCvec));
         
        int pos = findSz((char*)&HeadCvec[0], "\r\n\r\n", HeadCvec.size(), 4);
        //cout << "header pos = " << pos << endl;
        if(pos != -1)
        {
        	//cout << "recv http head seccess!" << endl;
        	
        	BodyCvec.assign(&HeadCvec[pos + 4], &(*HeadCvec.end()));
        	HeadCvec.assign(&HeadCvec[0], &HeadCvec[pos + 4]);
        	
            break;	
        }
        
        if(i > 10000)
        {
        	cout << "recv hearder error!" << endl;
            return;	
        }
    }
    
    string sRecvHead(HeadCvec.begin(), HeadCvec.end());
    string sFirstLine(sRecvHead, 0, 10);
    
    if(sFirstLine.find("GET") != string::npos)
    {
        do_get(newfd, sRecvHead);	
    }
    else if(sFirstLine.find("POST") != string::npos)
    {
        do_post(newfd, sRecvHead, BodyCvec);
    }
}

void HttpServer::onThreadCreate(pthread_t tid)
{
	
}

void HttpServer::do_get(int newfd, const string &sHead)
{
    /*
    string sVPath;
    getVPath(sVPath, sHead, GET);
    
    cout << "vpath: " << sVPath << endl;
    
    URLContainer urlArgMap;
    parserUrl(urlArgMap, sVPath);
    
    string sPageType(getMapValue(urlArgMap, string(VPATH)));
    cout << sPageType << endl;
    
	HttpHeader inputHeader;
	parseRequestHeader(sHead, inputHeader);

   //bool needkencode = false;
    if(sPageType == "ganeralhmacmd5.do")
    {
    	//needkencode = true;
    	
    	string sVar = getMapValue(urlArgMap, string("var"));
    	cout << "sVar = " << sVar << endl;
    	
    	vector<uint8_t> cvecVPath;
    	KEncode ke;
    	ke.DecodeFromChar(sVar, cvecVPath);
    	
    	cout << "cvecVPath =" << cvecVPath << endl;
    	
    	sVPath.assign(cvecVPath.begin(), cvecVPath.end());
    	while(sVPath.empty() == false)
    	{
    	    if(sVPath[0] == '/')
    	    {
    	        sVPath.erase(0, 1);
    	    }
    	    else
    	    {
    	        break;	
    	    }
    	}
    	
    	cout << "new VPath is: " << sVPath << endl;
    	
    	parserUrl(urlArgMap, sVPath);
    	cout << "new urlArgMap size is: " << urlArgMap.size() << endl;
    	
    	sPageType = getMapValue(urlArgMap, string(VPATH));
    	cout << "new sPageType = " << sPageType << endl;
    }

    if(sPageType == "login.do")
    {
		HttpHeader hearder;
		hearder.insert("Status", "200");
		
        string sXml;
        doLogin(sXml, hearder, inputHeader, urlArgMap);
        sendHttpData(newfd, sXml, &hearder);
    }
    else
    {
        // 下载文件
        string sName(getMapValue(urlArgMap, string(VPATH)));

        if(sName.empty() == true)
        {
			HttpHeader hearder;
			hearder.insert("Status", "404");
        	string sContent("404");
            sendHttpData(newfd, sContent, &hearder);
            return;
        }
        
        const time_t t1 = time(NULL);
        
        string sFileName = VHEAD + sName;
        cout << "sFileName = " << sFileName << endl;
        
        vector<uint8_t> cvec;
        if(ReadFile(cvec, sFileName) == true)
        {
			HttpHeader hearder;
			hearder.insert("Status", "200");
        	sendHttpData(newfd, cvec, &hearder);
        }
        else
        {
			HttpHeader hearder;
			hearder.insert("Status", "404");
        	string sContent("404");
            sendHttpData(newfd, sContent, &hearder); 
        }
        
        const time_t t2 = time(NULL);
        
        cout << "used time: " << t2 - t1 << endl;
    }
    */
}

void HttpServer::do_post(int newfd, const string &sHead, vector<uint8_t> &BodyCvec)
{
	// 接收body
	string sContentLength;
	SubstrF(sContentLength, sHead, "Content-Length:", "\r\n");

	size_t iLength = strtoul(sContentLength.c_str(), NULL, 10);
	char szBuff1[BUFFER_SIZE];
	while(BodyCvec.size() < iLength)
	{
		pollfd pollfd;
		pollfd.fd = newfd;
		pollfd.events = POLLIN | POLLHUP | POLLPRI;

		int retval = poll(&pollfd, 1, 10 * 1000);
        if(retval < 0)
        {
        	cout << "poll error" << endl;
       	    break;
        }
        else if(retval == 0)
        {
        	cout << "no data in the 3 second" << endl;
            break;
        }	
    
        memset(szBuff1, 0, BUFFER_SIZE);

        int iBytes = recv(newfd, szBuff1, BUFFER_SIZE, 0);
        if(iBytes < 0)
        {
            cerr << "Can't recv http head" << endl;
            break;
        }
        else if(iBytes == 0)
        {
            cerr << "peer closed!" << endl;
            break;
        }
        
        copy(szBuff1, szBuff1 + iBytes, back_inserter(BodyCvec));	
	}
	
	cout << "revc seccess!" << endl;
	
    string sVPath;
    getVPath(sVPath, sHead, POST);
    
	URLContainer urlArgMap;
    parserUrl(urlArgMap, sVPath);
    
    const string sPageType(getMapValue(urlArgMap, string(VPATH)));
    
    cout << "PageType: "<< sPageType << endl;
    
    // ungzip
    if(BodyCvec.size() > 3)
    {
    	vector<uint8_t> BodyCvec1(BodyCvec.begin(), BodyCvec.end());
    	if(BodyCvec1[0] == 0x1f && BodyCvec1[1] == 0x8b && BodyCvec1[2] == 0x08)
    	{
            ungzipxp(&BodyCvec1[0], BodyCvec1.size(), *(vector<uint8_t>*)&BodyCvec);
        }
    }
    
	HttpHeader inputHeader;
	parseRequestHeader(sHead, inputHeader);
    
    if(sPageType == "do_sql")
    {
		HttpHeader hearder;
		hearder.insert("Status", "200");
    	string sXml;
        m_data_center.do_sql(sXml, BodyCvec);
        sendHttpData(newfd, sXml, &hearder);
    }
  
    else
    {
		HttpHeader hearder;
		hearder.insert("Status", "404");
		
    	string sXml("404");
        sendHttpData(newfd, sXml, &hearder);
    }
}
 

pair<string, string> HttpServer::divideArg(const string &url, const string &sDelem)
{
	string::size_type pos = url.find(sDelem);
	if(pos != string::npos)
	{
	    return make_pair(url.substr(0, pos), url.substr(pos + sDelem.size()));
	}
	else
	{
		return make_pair("", "");
	}
}

void HttpServer::parserUrl(URLContainer &urlArgMap, const string &sVPath)
{
	urlArgMap.clear();
	
    string::size_type pos1 = sVPath.find("?");
    if(pos1 == string::npos)
    {
    	urlArgMap.insert(URLContainer::value_type(string(VPATH), sVPath));
    }
    else
    {
    	urlArgMap.insert(URLContainer::value_type(string(VPATH), sVPath.substr(0, pos1)));
    	
    	string::size_type pos2 = pos1 + 1;
    	string::size_type pos3 = pos1 + 1;
    	while(true)
    	{
    	    pos3 = sVPath.find("&", pos2);
    	    if(pos3 != string::npos)
    	    {
    	    	string sArgSec(sVPath, pos2, pos3 - pos2);
    	    	
    	    	urlArgMap.insert(divideArg(sArgSec, "="));
    	    	pos2 = pos3 + 1;
    	    }
    	    else
    	    {
    	    	string sArgSec(sVPath, pos2);
    	    	urlArgMap.insert(divideArg(sArgSec, "="));
    	    	break;    	    	
    	    }
    	}
    }
}

pair<int, int> HttpServer::getRang(const string &sRecv)
{
    string::size_type pos = sRecv.find("Range:");
    if(pos == string::npos)
    {
        return make_pair(-1, -1);
    }
    
    string sRang;
    Substr(sRang, sRecv, "Range:", "\r\n", pos);
    
    string sBegin;
    pos = Substr(sBegin, sRang, "bytes=", "-");
    if(pos == string::npos)
    {
        return make_pair(-1, -1);
    }
    
    const int beginPos = atoi(sBegin.c_str());
    
    string sEnd(sRang, pos);
    if(isStringDigit(sEnd) == false)
    {
        return make_pair(beginPos, -1);
    }
    
    const int endPos = atoi(sEnd.c_str());
    
    return make_pair(beginPos, endPos);
}

inline void HttpServer::getVPath(string &sVPath, const string &sContent, const HTTP_TYPE type)
{
	sVPath.clear();
	
	if(type == GET)
	{
		Substr(sVPath, sContent, "GET ", " HTTP");
	}
	else if(type == POST)
	{
		Substr(sVPath, sContent, "POST ", " HTTP");
	}
	else
	{
	    return;	
	}
	
	if(sVPath.size() < 7)
	{
	    return;	
	}
	
	// 虚拟路径是完整url的情况, 去http头
	if(sVPath.substr(0, 7) == "http://")
	{
	    string::size_type pos = sVPath.find('/', 7);
	    if(pos != string::npos)
	    {
	    	sVPath.erase(0, pos);
	    }
	    else
	    {
	    	// 访问主页的情况
	    	sVPath.clear();
	        return;	
	    }
	}
	
	// 去掉多'/'的情况
	while(sVPath[0] == '/' || sVPath[0] == ' ')
    {
    	sVPath.erase(0, 1);
    }

//    cout << "old vpath: " << sVPath << endl;
	
	// 过滤&amp;的情况
	const string sKeyWord("&amp;");
	while(true)
	{
	    string::size_type pos = sVPath.find(sKeyWord);
	    if(pos != string::npos)
	    {
	    	sVPath.replace(pos, sKeyWord.size(), "&");
	    }
	    else
	    {
	        break;	
	    }
	}
}

int HttpServer::getContentLength(const string &s)
{	
	string sLength;
	if(Substr(sLength, s, "Content-Length:", "\r\n") == string::npos)
	{
		return -1;
	}
	
	string::size_type pos = 0;
	while(true)
	{
	    pos = sLength.find(" ");
	    if(pos == string::npos)
	    {
	    	break;
	    }
	    
	    sLength.erase(pos, 1);
	    pos = 0;
	}
	
	return atoi(sLength.c_str());
}

void HttpServer::makeRespHeader(string &s, size_t contentLength, const HttpHeader *header) const
{
	if(header == NULL)
	{
		s = "HTTP/1.1 200 OK\r\n"
			"Content-Length: " + lexical_cast<string>(contentLength) + "\r\n";
		return;
	}

	string sStatus = (*header)["Status"];
	int status = atoi(sStatus.c_str());

	string sSuffix;
	switch(status)
	{
		case 404:
			sSuffix = "Not Found";
			break;

		case 200:
		default:
			sSuffix = "OK";
			break;
	}
    
    bool hasUa = false;
	s = "HTTP/1.1 " + (*header)["Status"] + " " + sSuffix + "\r\n"
		"Content-Length: " + lexical_cast<string>(contentLength) + "\r\n";
	if(header != NULL)
	{
		for(size_t i = 0; i < header->m_HttpheaderVec.size(); ++i)
		{
			const HttpHeaderItem &headeritem = header->m_HttpheaderVec[i];

			if(headeritem.m_sKey != "VPath")
			{
				s += headeritem.m_sKey + ": " + headeritem.m_sValue + "\r\n";
			}
			else if(headeritem.m_sKey != "User-Agent")
			{
				hasUa = true;
			}
		}
	}
	
	if(!hasUa)
	{
		s += "User-Agent: Scan Universe Server 1.0\r\n";
	}

	s += "\r\n";
}

int HttpServer::sendHttpData(int newfd, const vector<uint8_t> &cvec, const HttpHeader *header, bool needkencode)
{
	vector<uint8_t> cencode;
	if(needkencode == true)
	{
	    KEncode ke;
	    ke.Encode(cvec, cencode);
	}
	else
	{
		cencode.assign(cvec.begin(), cvec.end());
	}
	
	string s;
	makeRespHeader(s, cvec.size(), header);
    // 发送大小
    size_t numbytes = 0;
    while(numbytes < s.size())
    {
    	int rv = poll_send(newfd, s.c_str() + numbytes, s.size() - numbytes, 20);
    	if(rv < 0)
    	{
	        printf("[1, Send]ERROR: errno = %d, strerror = %s \n", errno, strerror_s(errno).c_str());
    	    return -1;	
    	}

    	numbytes += rv;
        cout << "send head " << numbytes << " bytes" << endl;
    }
    //time_t t2 = time(NULL);

    // 发送数据
    numbytes = 0;
    for(int i = 0; numbytes < cencode.size(); ++i)
    {
    	int rv = send(newfd, &cencode[numbytes], cencode.size() - numbytes, 0);
    	
    	//time_t t22 = time(NULL);
    	if(rv < 0)
    	{
	        printf("[2, Send]ERROR: errno = %d, strerror = %s \n", errno, strerror_s(errno).c_str());
    	    return -2;	
    	}
    	
    	numbytes += rv;
        cout << numbytes << " bytes sent" << endl;
    }
    //time_t t3 = time(NULL);

    cout << "send success!" << endl;
    return numbytes;
}

int HttpServer::sendHttpData(int newfd, const string &sContent, const HttpHeader *header, bool needkencode)
{
	vector<uint8_t> cvec(sContent.begin(), sContent.end());
	return sendHttpData(newfd, cvec, header, needkencode);
}

void HttpServer::parseRequestHeader(const string &sHead, HttpHeader &mapHeader) const
{
	mapHeader.clear();
	vector<string> vHeaders;
	SplitStringtoVectorString(sHead, "\r\n", vHeaders);
	for(unsigned int i = 0; i < vHeaders.size(); ++i)
	{
		string sLine = vHeaders[i];
		vector<string> vSubHeaders;
		SplitStringtoVectorString(sLine, ":", vSubHeaders);
		if(vSubHeaders.size() == 2)
		{
			trim(vSubHeaders[1]);
			mapHeader.insert(vSubHeaders[0], vSubHeaders[1]);
		}
	}
}
