// check range out

/*
 *  Http.cpp
 *  TGBaidu
 *
 *  Created by xiapeng on 11-4-26.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "Http.h"

/*-----------------------------------------------------------------------*/
void Post11FormItem::clear()
{
    m_sName = "";
    m_sFileName = "";
    m_sAction = "";
    m_sContentType = "";
    m_cvec.clear();
    m_sBoundary.clear();
}

void Post11FormItem::show(ostream &os) const
{
    LOG("========Post11FormItem========");
    LOG("m_sName = " +  m_sName);
    LOG("m_sFileName = " + m_sFileName);
    LOG("m_sAction = " + m_sAction);
    LOG("m_sContentType = " + m_sContentType);
    LOG("m_cvec.size() = " + m_cvec.size());
    LOG("m_sBoundary = " + m_sBoundary); 
}


/*-----------------------------------------------------------------------*/
HttpClient::HttpClient()
{
	m_sockfd = 0;
}

HttpClient::HttpClient(const ProxyBag &bag)
{
    m_bag = bag;
}

HttpClient::~HttpClient()
{
//    WSACleanup();
}

int HttpClient::Get2(const URL &url, vector<uint8_t> &cvec, const HttpHeader *pInputHeader, HttpHeader *pheader, long timeout)
{
    TraceLog log("Get2");
    cout << "url: " << url.wholeUrl() << endl;
	cvec.clear();
	
    string sIp;
	if(Connect(url.host().c_str(), url.port(), sIp) == false)
	{
		close(m_sockfd);
	    return -1;
	}
	
	cout << "use ip: " << sIp << endl;
	
	//	g_LOG << "connect ok!" << endl;
	//	g_LOG << "sockfd = " << m_sockfd << endl;
	
	string s;
	makeGetRequestHead(s, url, pInputHeader);
	
	//cout << "===========================" << endl;
	//cout << s << endl;
	//cout << "===========================" << endl;
	
	unsigned int numbytes = 0;
	while(numbytes < s.size())
	{
	    int rv = send(m_sockfd, s.c_str() + numbytes, s.size() - numbytes, 0);
	    if(rv < 0)
	    {
			cout << "send error" << endl;
			close(m_sockfd);
			return -2;
	    }
		
		numbytes += rv;
		//		g_LOG << "已经发送了：" << numbytes << "字节" << endl;
	}
	
	//	g_LOG << "发送完成！" << endl;
	
	vector<uint8_t> cvecWithHead;
	int pos = recvHttpData2(cvecWithHead, timeout);	
	if(pos < 0 || cvecWithHead.empty() == true)
	{
		close(m_sockfd);
	    return -3;
	}
	
	if(pheader != NULL)
	{
		const string sHead(&cvecWithHead[0], &cvecWithHead[0] + pos);
		makeRespHead(sHead, *pheader);
	}
	
	cvec.assign(&cvecWithHead[0] + pos, &cvecWithHead[0] + cvecWithHead.size());
    if(cvec.size() >= 3)
    {
	    string sFirst3chars(cvec.begin(), cvec.begin() + 3);
	    if(sFirst3chars == "\x1F\x8B\x08")
	    {
	    	vector<uint8_t> unzipedData;
	    	
	    	// ungzip
	    	ungzipxp(&cvec[0], cvec.size(), unzipedData);
	    	cvec.swap(unzipedData);;
	    }
    }
	
	close(m_sockfd);
	return 0;
}

int HttpClient::Post11(const URL &url, const vector<Post11FormItem> &data, vector<uint8_t> &cvec, const HttpHeader *pInputHeader, HttpHeader *pheader, long timeout)
{
	//cout << "url: " << url.wholeUrl() << endl;
	
	cvec.clear();
	
    string sIp;
	if(Connect(url.host().c_str(), url.port(), sIp) == false)
	{
		close(m_sockfd);
	    return -1;
	}
		
	vector<uint8_t> senddata1;
	for(unsigned int i = 0; i < data.size(); ++i)
    {
        string sValue = "--" + data[i].m_sBoundary + "\r\n";
		for(size_t j = 0; j < sValue.size(); ++j)
        {
		    senddata1.push_back(sValue[j]);
		}

        if(data[i].m_sContentType == "image/jpeg")
        {
            sValue = "Content-Disposition:form-data; name=\"" + data[i].m_sName + "\"; ";
            for(size_t j = 0; j < sValue.size(); ++j)
			{
				senddata1.push_back(sValue[j]);
			}
			
            sValue = "filename=\"" + data[i].m_sFileName + "\"\r\n";
            for(size_t j = 0; j < sValue.size(); ++j)
			{
				senddata1.push_back(sValue[j]);
			}

            sValue = "Content-Type:" + data[i].m_sContentType + "\r\n\r\n";
            for(size_t j = 0; j < sValue.size(); ++j)
			{
				senddata1.push_back(sValue[j]);
			}

            for(size_t j = 0; j < data[i].m_cvec.size(); ++j)
			{
				senddata1.push_back(data[i].m_cvec[j]);
			}
			
            string sEnd("\r\n");
			for(size_t j = 0; j < sEnd.size(); ++j)
			{
				senddata1.push_back(sEnd[j]);
			}
        }
        else if(data[i].m_sContentType == "")
        {
            sValue = "Content-Disposition:form-data;name=\"" + data[i].m_sName + "\";";
            for(size_t j = 0; j < sValue.size(); ++j)
			{
				senddata1.push_back(sValue[j]);
			}
			
            sValue = "filename=\"" + data[i].m_sFileName + "\"\r\n\r\n";
            for(size_t j = 0; j < sValue.size(); ++j)
			{
				senddata1.push_back(sValue[j]);
			}

            for(size_t j = 0; j < data[i].m_cvec.size(); ++j)
			{
				senddata1.push_back(data[i].m_cvec[j]);
			}
			
            string sEnd("\r\n");
			for(size_t j = 0; j < sEnd.size(); ++j)
			{
				senddata1.push_back(sEnd[j]);
			}
        }
        else
        {
            sValue = "Content-Disposition:form-data;name=\"" + data[i].m_sName + "\"\r\n";
            for(size_t j = 0; j < sValue.size(); ++j)
			{
				senddata1.push_back(sValue[j]);
			}

            sValue = "\r\n" + data[i].m_sAction + "\r\n";
			for(size_t j = 0; j < sValue.size(); ++j)
			{
				senddata1.push_back(sValue[j]);
			}
        }
        
        if(i == data.size() - 1)
        {
        	senddata1.push_back('-');
        	senddata1.push_back('-');
			for(size_t j = 0; j < data[i].m_sBoundary.size(); ++j)
			{
			    senddata1.push_back(data[i].m_sBoundary[j]);
			}
			senddata1.push_back('-');
        	senddata1.push_back('-');
        }
    }
	
	string s;
	makePostRequestHead(s, url, pInputHeader, senddata1);
	
//	cout << "==========================================" << endl;
//	cout << s << endl;
//	cout << senddata1 << endl;
//	cout << "==========================================" << endl;

	vector<uint8_t> senddata;
    for(size_t i = 0; i < s.size(); ++i)
	{
	    senddata.push_back(s[i]);
	}

    for(size_t i = 0; i < senddata1.size(); ++i)
	{
	    senddata.push_back(senddata1[i]);
	}
	
    //LOG(senddata);
    
    //writeFile(senddata, "/root/xmdx/xmsf/111.log");
	
	unsigned int numbytes = 0U;
	while(numbytes < senddata.size())
	{
	    int rv = send(m_sockfd, &senddata[0] + numbytes, senddata.size() - numbytes, 0);
	    if(rv < 0)
	    {
			LOG("send error");
			close(m_sockfd);
			return -2;
	    }
		
		numbytes += rv;
		LOG(lexical_cast<string>(numbytes) + "bytes send");
	}
	
	LOG("send seccess!");
    
	vector<uint8_t> cvecWithHead;
	int pos = recvHttpData2(cvecWithHead, timeout);
	if(pos < 0)
    {
        cout << "recvHttpData error: " << pos << endl;
        return -3;
    }
	if(cvecWithHead.empty() == true)
	{
		close(m_sockfd);
	    return -4;
	}
	
	if(pheader != NULL)
	{
		const string sHead(&cvecWithHead[0], &cvecWithHead[0] + pos);
		makeRespHead(sHead, *pheader);
	}
	
	cvec.assign(&cvecWithHead[0] + pos, &cvecWithHead[0] + cvecWithHead.size());
    if(cvec.size() >= 3)
    {
	    string sFirst3chars(cvec.begin(), cvec.begin() + 3);
	    if(sFirst3chars == "\x1F\x8B\x08")
	    {
	    	vector<uint8_t> unzipedData;
	    	
	    	// ungzip
	    	ungzipxp(&cvec[0], cvec.size(), unzipedData);
	    	cvec.swap(unzipedData);;
	    }
    }
	
	close(m_sockfd);
	return 0;	
}

bool HttpClient::Connect(const char *domain1, const int port1, string &sIp)
{
	//cout << "Connect!" << endl;
    //cout << "++++domain1: " << domain1 << endl;
    //cout << "++++port1: " << port1 << endl;

	m_sockfd = socket(AF_INET, SOCK_STREAM, 0);  
	if(m_sockfd < 0) 
	{
		cout << "socket error: " << strerror(errno) << endl;
		return false;
	}

	string sDomain;
	int port = 0;
	if(m_bag.empty())
	{
		//cout << "bag is empty!" << endl;
	    sDomain = domain1;
		port = port1;
	}
	else
	{
		//cout << "using proxy mode!" << endl;
		//cout << "domain1: " << domain1 << endl;
        //m_bag.show();
 
	    sDomain = m_bag.m_sDomain;
        port = m_bag.m_port;
	}

	//cout << "++++sDomain = " << sDomain << endl;
	//cout << "++++port = " << port << endl;
	
	sockaddr_in me;
	memset(&me, 0, sizeof(sockaddr_in));    
	me.sin_family = AF_INET;
	me.sin_port = htons(port);
	if(isIpType(sDomain.c_str()) == false)
	{
		//cout << "is not ip" << endl;
		hostent *site = gethostbyname(sDomain.c_str());
		if(site == NULL)
		{
			//cout << "gethostbyname error!" << endl;
			return false;
		}
		
		for(char **p = site->h_addr_list; *p != NULL; ++p)
		{
			if(sIp == "")
			{
				sIp = inet_ntoa(*(in_addr *)(*p));
			}
			
			//LOG(string("ip: ") + inet_ntoa(*(in_addr *)(*p)));
		}
		
		memcpy(&me.sin_addr, site->h_addr_list[0], site->h_length);
	}
	else
	{
		//cout << "is ip" << endl;
		sIp = sDomain;
        inet_aton(sDomain.c_str(), &me.sin_addr);
	}
	
	//cout << "sIp: " << sIp << endl;
	
	int iMode = 1;
	if(ioctl(m_sockfd, FIONBIO, &iMode))
	{
		printf("ioctl to no block error!\n");
		return false;
	}
    
    //printf("ioctl to no block success!\n");
    
	connect(m_sockfd, reinterpret_cast<sockaddr*>(&me), sizeof(sockaddr));
	
	pollfd pollfd;
	pollfd.fd = m_sockfd;
	pollfd.events = POLLOUT;
	
    int retval = poll(&pollfd, 1, 20 * 1000);
	if(retval <= 0)
	{
		printf("connect time out, error!\n");
		return false;
	}
	
	int error = -1;
	socklen_t len = sizeof(socklen_t);
	getsockopt(m_sockfd, SOL_SOCKET, SO_ERROR, &error, &len);
	if(error != 0)
	{
		printf("socket opt error!\n");
		return false;
	}
	
	iMode = 0;
	int iResult = ioctl(m_sockfd, FIONBIO, &iMode);
	if(iResult)
	{
		printf("ioctl to block error!\n");
		return false;
	}
    
   // printf("ioctl to block success!\n");
    
    
    if(m_bag.empty() == false)
	{
		bool proxysuccess = false;
		switch(m_bag.m_type)
	    {
	        case ProxyBag::EHTTPS:
	        {
	        	proxysuccess = doConnectHttps(domain1, port1);
	        	break;
	        }
	        	
	        case ProxyBag::ESOCK5:
	        {
	        	proxysuccess = doConnectSock5(domain1, port1);
	        	break;
	        }
	        		
	        default:
	        	break;	
	    }
		
		if(proxysuccess == false)
		{
			return false;
		}
	}
	
//	timeval timeout;
//    timeout.tv_sec = 20;    
//    timeout.tv_usec = 0;
//    iResult = setsockopt(m_sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout.tv_sec, sizeof(struct timeval));
//    if(iResult < 0)
//    {
//        perror("setsockopt SO_RCVTIMEO error");
//        return false;
//    }
    
	//cout << "connect success!" << endl;
	return true;
}

bool HttpClient::doConnectHttps(const string &sServerName, uint16_t port1)
{
    string sMsg = "CONNECT " + sServerName + ":" + lexical_cast<string>(port1) + " HTTP/1.0\r\n"
                  "Host: " + sServerName + "\r\n"
                  "User-Agent: webclient\r\n\r\n";
	int sockfd = m_sockfd;
	if(send(sockfd, sMsg.c_str(), sMsg.size(), 0) == -1)
	{
		cout << "proxy send error!" << endl;
		return false;
	}
	
	pollfd pollfd;
	pollfd.fd = sockfd;
	pollfd.events = POLLIN | POLLHUP | POLLPRI;
	
    int retval = poll(&pollfd, 1, 20 * 1000);
	if(retval < 0)
	{
		cout << "poll error" << endl;
		return false;
	}
	else if(retval == 0)
	{
		cout << "no data in the 20 second" << endl;
		return false;
	}
	
	char szBuff[4096] = "";
	ssize_t iBytes = recv(sockfd, szBuff, 4096, 0);
	if(iBytes == -1)
	{
		cout << "proxy recv error!" << endl;
		return false;
	}
	
	if(strstr(szBuff, "200") == NULL)
	{
		cout << "proxy status error!" << endl;
		return false;
	}
	
	return true;
}

bool HttpClient::doConnectSock5(const string &sServerName, uint16_t port1)
{
	cout << "sServerName: " << sServerName << endl;
	bool isIpType = ::isIpType(sServerName.c_str());
	
	char szBuff[BUFFER_SIZE] = "";
	char szSend[3] = {0x05, 0x01, 0x00};
	showHex(szSend, sizeof(szSend));
	
	int sockfd = m_sockfd;
	if(send(sockfd, szSend, 3, 0) < 0)
	{
		cout << "proxy connect send failed!" << endl;
		return false;
	}
	
	int bytes = poll_recv(sockfd, szBuff, BUFFER_SIZE, 30);
	if((bytes <= 0))
	{
		cout << "proxy connect recv failed!" << endl;
		return false;
	}
	
	showHex(szBuff, bytes);
	if(szBuff[0] != 0x05 || szBuff[1] != 0x00)
	{
		cout << "proxy connect recv failed1: ";
		showHex(szBuff, bytes);
		cout << endl;
		return false;
	}
	
	unsigned int inaddr = inet_addr(sServerName.c_str());
	
	unsigned char *p = (unsigned char*)&inaddr;
	printf("%d.%d.%d.%d\n", p[0], p[1], p[2], p[3]);
	
	vector<uint8_t> sendvec;
	sendvec.push_back(0x05);
	sendvec.push_back(0x01);
	sendvec.push_back(0x00);
	
	
	if(isIpType)
	{
		sendvec.push_back(0x01);
		
		sendvec.push_back(p[0]);
		sendvec.push_back(p[1]);
		sendvec.push_back(p[2]);
		sendvec.push_back(p[3]);
	}
	else
	{
		sendvec.push_back(0x03);
		sendvec.push_back(sServerName.size());
		copy(sServerName.begin(), sServerName.end(), back_inserter(sendvec));
	}
	
	sendvec.push_back(((char*)&port1)[1]);
	sendvec.push_back(((char*)&port1)[0]);
	
	//char szSendBuff1[] = {0x05, 0x01, 0x00, 0x01, p[0], p[1], p[2], p[3], ((char*)&port1)[1], ((char*)&port1)[0]};
	showHex((char *)&sendvec[0], sendvec.size());
    if(send(sockfd, &sendvec[0], sendvec.size(), 0) < 0)
	{
		cout << "proxy connect send 2 failed!" << endl;
		return false;
	}
	
	memset(szBuff, 0, BUFFER_SIZE);
	bytes = poll_recv(sockfd, szBuff, BUFFER_SIZE, 30);
	if((bytes <= 0))
	{
		cout << "proxy connect recv 2 failed!" << endl;
		return false;
	}
	
	showHex(szBuff, bytes);
	
	if(bytes >= 10)
	{
		uint16_t port = 0;
		char *p = (char*)&port;
		p[0] = szBuff[9];
		p[1] = szBuff[8];
		printf("vps ip:port = [%u.%u.%u.%u:%u]\n", szBuff[4] & 0xffu, szBuff[5] & 0xffu, szBuff[6] & 0xffu, szBuff[7] & 0xffu, port & 0xffffu);
	}
	
	if(szBuff[1] == 0x01)
	{
		cout << "proxy connect recv 3 failed!" << endl;
		return false;
	}
	
	return true;
}


void HttpClient::setBag(const ProxyBag &bag)
{
	m_bag = bag;
}

int HttpClient::getContentLength(const string &sori) const
{
	string s(sori);
	StringToLower(s);
	
	string sLength;
	if(Substr(sLength, s, "content-length:", "\r\n") == string::npos)
	{
		return -1;
	}
	
	string::size_type pos = 0U;
	while(true)
	{
		pos = sLength.find(" ");
		if(pos == string::npos)
		{
			break;
		}
		
		sLength.erase(pos, 1);
	}
	
	return atoi(sLength.c_str());
}

int HttpClient::recvHttpData(string &sRecv, long timeout)
{
	char szBuff1[BUFFER_SIZE];
	
	int iContentLength = -1;
	
	string::size_type iBodyPos = string::npos;
	while(true)
	{	
		pollfd pollfd;
		pollfd.fd = m_sockfd;
		pollfd.events = POLLIN | POLLHUP | POLLPRI;
		
	    int retval = poll(&pollfd, 1, timeout * 1000);
		if(retval < 0)
		{
			LOG("poll error");
			return -1;
		}
		else if(retval == 0)
		{
			LOG("no data in the 20 second");
			return -3;	
		}
		
		memset(szBuff1, 0, BUFFER_SIZE);
		
		int iBytes = recv(m_sockfd, szBuff1, BUFFER_SIZE, 0);
        if(iBytes == 0)
        {
            LOG("peer closed");
			break;
        }
        
		if(iBytes < 0)
		{
			LOG("read error");
			return -1;
		}
		
		for(size_t i = 0; i < sRecv.size(); ++i)
		{
			sRecv += szBuff1[i];
		}

		if(iContentLength == -1)
		{
			// find content-length
			if((iBodyPos = sRecv.find("\r\n\r\n")) != string::npos)
			{
				//LOG("http head recv seccess!");
				
				// Content-Length
				iContentLength = getContentLength(sRecv);
				if(iContentLength == -1)
				{
					LOG("Can't find Content-Length");
					return -2;
				}
			}
			else
			{
				// not complete for http head
				continue;	
			}
		}
		
		if(sRecv.size() >= iBodyPos + 4 + iContentLength)
		{
			LOG("recv seccess!");
			break;	
		}
	}
	
	return iBodyPos + 4;
}

int HttpClient::recvHttpData(vector<uint8_t> &cvec, long timeout)
{
	char szBuff1[BUFFER_SIZE];
	int iContentLength = -1;
	string::size_type iBodyPos = string::npos;
	for(int i = 0; true; ++i)
	{	
		pollfd pollfd;
		pollfd.fd = m_sockfd;
		pollfd.events = POLLIN | POLLHUP | POLLPRI;
		
	    int retval = poll(&pollfd, 1, timeout * 1000);
		if(retval < 0)
		{
			LOG("poll error");
			return -1;
		}
		else if(retval == 0)
		{
			LOG("no data in the 20 second");
			return -3;	
		}
		
		memset(szBuff1, 0, BUFFER_SIZE);
		
		int iBytes = recv(m_sockfd, szBuff1, BUFFER_SIZE, 0);
        if(iBytes == 0)
        {
            LOG("peer closed");
			break;
        }
        
		if(iBytes < 0)
		{
			LOG("read error");
			return -1;
		}
		
		for(int j = 0; j < iBytes; ++j)
		{
			cvec.push_back(szBuff1[j]);
		}
		LOG("recv: " + lexical_cast<string>(iBytes));
        //copy(szBuff1, szBuff1 + iBytes, ostreambuf_iterator<char>(cout));
        //LOG(endl;
		
		if(iContentLength == -1)
		{
			// find content-length
			int pos = findSz((char*)&cvec[0], "\r\n\r\n", cvec.size(), 4);
			if(pos != -1)
			{
				//LOG("http head recv seccess!");
				
				// Content-Length
				iBodyPos = pos;
				string sHead(cvec.begin(), cvec.begin() + pos + 4);
				LOG(sHead);
				iContentLength = getContentLength(sHead);
				if(iContentLength == -1 || i > 10)
				{
					LOG("Can't find Content-Length");
					return -2;
				}
			}
			else
			{
				// not complete for http head
				continue;	
			}
		}
		
		if(cvec.size() >= iBodyPos + 4 + iContentLength)
		{
			LOG("recv seccess!");
			break;	
		}
	}
	
	return iBodyPos + 4;
}

int HttpClient::recvHttpData(vector<uint8_t> &cvec, const string &sFileName, long timeout)
{
	ofstream out(sFileName.c_str(), ios::binary);
	if(!out)
	{
		LOG(sFileName + " can not open!");
	    return -1;	
	}
	
	char szBuff1[BUFFER_SIZE];
	int iContentLength = -1;
	int iTotalBodyBytes = 0;
	string::size_type iBodyPos = string::npos;
	for(int i = 0; true; ++i)
	{	
		pollfd pollfd;
		pollfd.fd = m_sockfd;
		pollfd.events = POLLIN | POLLHUP | POLLPRI;
		
	    int retval = poll(&pollfd, 1, timeout * 1000);
		if(retval < 0)
		{
			LOG("poll error");
			return -1;
		}
		else if(retval == 0)
		{
			LOG("no data in the 20 second");
			return -3;	
		}
		
		memset(szBuff1, 0, BUFFER_SIZE);
		
		int iBytes = recv(m_sockfd, szBuff1, BUFFER_SIZE, 0);
        if(iBytes == 0)
        {
            LOG("peer closed");
			break;
        }
        
		if(iBytes < 0)
		{
			LOG("read error");
			return -1;
		}
		
		if(iContentLength == -1)
		{
			for(int j = 0; j < iBytes; ++j)
			{
			    cvec.push_back(szBuff1[j]);
			}
		    LOG("recv: " + lexical_cast<string>(iBytes));
		    
			// find content-length
			int pos = findSz((char*)&cvec[0], "\r\n\r\n", cvec.size(), 4);
			if(pos != -1)
			{
				//LOG("http head recv seccess!");
				// Content-Length
				iBodyPos = pos;
				string sHead(cvec.begin(), cvec.begin() + pos + 4);
				LOG(sHead);
				iContentLength = getContentLength(sHead);
				if(iContentLength == -1 || i > 10)
				{
					LOG("Can't find Content-Length");
					return -2;
				}
				
				copy(cvec.begin() + pos + 4, cvec.end(), ostreambuf_iterator<char>(out));
				iTotalBodyBytes = cvec.end() - (cvec.begin() + pos + 4);
			}
			else
			{
				// not complete for http head
				continue;	
			}
		}
		else
		{
			iTotalBodyBytes += iBytes;
			copy(szBuff1, szBuff1 + iBytes, ostreambuf_iterator<char>(out));
		    LOG("recv: " + lexical_cast<string>(iBytes));	
		}
		
		if(iTotalBodyBytes >= iContentLength)
		{
			LOG("recv seccess!");
			break;	
		}
	}
	
	return iBodyPos + 4;
}

int HttpClient::recvHttpData2(vector<uint8_t> &cvec, long timeout)
{
	// 单字节接受，直到接收完http头
	int i = 0;
	for(; i < 10000; ++i)
	{
		char szAchar[1] = "";
		pollfd pollfd;
		pollfd.fd = m_sockfd;
		pollfd.events = POLLIN | POLLHUP | POLLPRI;
		
	    int retval = poll(&pollfd, 1, timeout * 1000);
		if(retval < 0)
		{
			cout << "poll error" << endl;
			return -1;
		}
		else if(retval == 0)
		{
			cout << "no data in the 20 second" << endl;
			return -3;	
		}
		
		int iBytes = recv(m_sockfd, szAchar, 1, 0);
        if(iBytes == 0)
        {
            cerr << "peer closed" << endl;
			break;
        }
		else if(iBytes < 0)
		{
			cerr << "read error" << endl;
			return -1;
		}
		//cout << szAchar[0];
		cvec.push_back(szAchar[0]);
		
		if(cvec.size() >=4)
		{
			if(strncmp((char*)&(*(cvec.end() - 4)), "\r\n\r\n", 4) == 0)
			{
				//cout << "get head success!" << endl;
				break;
			}
		}
	}
	
	if(i >= 9999)
	{
		cout << "Recv head Error2" << endl;
		return -2;
	}
	
	const int iBodyPos = cvec.size();
	//cout << "iBodyPos = " << iBodyPos << endl;
	
	// 判断是Content_Length,还是truck
	string sHead(cvec.begin(), cvec.end());
	StringToLower(sHead);
	
	if(sHead.find("content-length:") != string::npos)
	{
		//cout << "content-length mode" << endl;
		const int iContentLength = getContentLength(sHead);
		if(iContentLength == -1)
		{
			cout << "Can't find Content-Length" << endl;
			return -3;
		}
		
		//cout << "content-length: " << iContentLength << endl;
		if(iContentLength <= 0)
		{
			return iBodyPos;
		}
		
		char szBuff1[BUFFER_SIZE];
		for(int i = 0; true; ++i)
		{	
			pollfd pollfd;
			pollfd.fd = m_sockfd;
			pollfd.events = POLLIN | POLLHUP | POLLPRI;
			
		    int retval = poll(&pollfd, 1, timeout * 1000);
			if(retval < 0)
			{
				cout << "poll error" << endl;
				return -1;
			}
			else if(retval == 0)
			{
				cout << "no data in the 20 second" << endl;
				return -3;	
			}
			
			memset(szBuff1, 0, BUFFER_SIZE);
			
			int iBytes = recv(m_sockfd, szBuff1, BUFFER_SIZE, 0);
	        if(iBytes == 0)
	        {
	            cerr << "peer closed" << endl;
				break;
	        }
	        
			if(iBytes < 0)
			{
				cerr << "read error" << endl;
				return -1;
			}
			
			for(int i = 0; i < iBytes; ++i)
			{
			    cvec.push_back(szBuff1[i]);
			}
			cout << "recv: " << iBytes << endl;
	        //copy(szBuff1, szBuff1 + iBytes, ostreambuf_iterator<char>(cout));
	        //cout << endl;
			
			if((int)cvec.size() >= iBodyPos + iContentLength)
			{
				cout << "recv seccess!" << endl;
				break;	
			}
		}
	}
	else if(sHead.find("transfer-encoding: chunked") != string::npos)
	{
		cout << "Transfer-Encoding: chunked mode" << endl;
		
		for(int cnt = 0; cnt < 10000000; ++cnt)
		{
			vector<uint8_t> chunckData;
			int rv = recvAChunck(chunckData, timeout);
			if(rv > 0)
			{
				for(unsigned int i = 0; i < chunckData.size(); ++i)
				{
					cvec.push_back(chunckData[i]);
				}
			}
			else if(rv == 0)
			{
				// end!
				break;
			}
			else
			{
				cout << "recv a chunck error: " << rv << endl;
				return -5;
			}
		} // end while
	}
	else
	{
		cout << "other mode" << endl;
	}
	
	return iBodyPos;
}

int HttpClient::recvAChunck(vector<uint8_t> &chunckData, long timeout)
{
	string sChunckLengthWithEnd;
	unsigned int iChunckLength = 0;
	for(int i = 0; i < 1000; ++i)
	{
		char szAchar[1] = "";
		
		pollfd pollfd;
		pollfd.fd = m_sockfd;
		pollfd.events = POLLIN | POLLHUP | POLLPRI;
		
	    int retval = poll(&pollfd, 1, timeout * 1000);
		if(retval < 0)
		{
			cout << "poll error" << endl;
			return -1;
		}
		else if(retval == 0)
		{
			cout << "no data in the 20 second" << endl;
			return -3;	
		}
		
		int iBytes = recv(m_sockfd, szAchar, 1, 0);
        if(iBytes == 0)
        {
            cerr << "peer closed" << endl;
			break;
        }
        
		if(iBytes < 0)
		{
			cerr << "read error" << endl;
			return -1;
		}
		
		sChunckLengthWithEnd += szAchar[0];

		if(sChunckLengthWithEnd.rfind("\r\n\r\n") != string::npos)
		{
			cout << "get head success!" << endl;
			break;
		}
		
		// 去掉开始的\r\n
		if(sChunckLengthWithEnd.size() >= 2)
		{
			string headBreak(sChunckLengthWithEnd.substr(0, 2));
			if(headBreak == "\r\n")
			{
				cout << "delete first change line." << endl;
				sChunckLengthWithEnd.erase(0, 2);
			}
		}
		
		cout << "sHexNum = " << sChunckLengthWithEnd << endl;
		
		string::size_type pos = sChunckLengthWithEnd.find("\r\n");
		if(pos != string::npos)
		{
			iChunckLength = strtol(sChunckLengthWithEnd.c_str(), NULL, 16);
			cout << "iChunckLength = " << iChunckLength << endl;
			
			break;
		} // end if
	} //end for
	
	if(iChunckLength <= 0)
	{
		// 结束
		return 0;
	}
	
    char szBuff1[BUFFER_SIZE];
	for(int i = 0; true; ++i)
	{
		const int ilast = iChunckLength - chunckData.size();
		
		pollfd pollfd;
		pollfd.fd = m_sockfd;
		pollfd.events = POLLIN | POLLHUP | POLLPRI;
		
	    int retval = poll(&pollfd, 1, timeout * 1000);
		
		if(retval < 0)
		{
			cout << "poll error" << endl;
			return -1;
		}
		else if(retval == 0)
		{
			cout << "no data in the 20 second" << endl;
			return -3;	
		}
		
		memset(szBuff1, 0, BUFFER_SIZE);
		
		int currecv = ilast;
		if(currecv > BUFFER_SIZE)
		{
		    currecv = BUFFER_SIZE;
		}
		
		int iBytes = recv(m_sockfd, szBuff1, currecv, 0);
        if(iBytes == 0)
        {
            cerr << "peer closed" << endl;
			break;
        }
        
		if(iBytes < 0)
		{
			cerr << "read error" << endl;
			return -1;
		}
		
        for(int i = 0; i < iBytes; ++i)
		{
		    chunckData.push_back(szBuff1[i]);
		}

		cout << "recv: " << iBytes << endl;
        //copy(szBuff1, szBuff1 + iBytes, ostreambuf_iterator<char>(cout));
       // cout << endl;
		
		cout << "chunckData.size() = " << chunckData.size() << endl;
		
		if(chunckData.size() >= iChunckLength)
		{
			cout << "recv A Thunck seccess!" << endl;			
			break;
		}
	}
	
	return chunckData.size();
}

int HttpClient::Post(const URL &url, const vector<uint8_t> &data, vector<uint8_t> &cvec, const HttpHeader *pInputHeader, HttpHeader *pheader, long timeout)
{
	//TraceLog log("Post");
	//cout << "url: " << url.wholeUrl() << endl;
	cvec.clear();
	
    string sIp;
	if(Connect(url.host().c_str(), url.port(), sIp) == false)
	{
		close(m_sockfd);
		return -1;
	}
	
	//LOG("use ip: " + sIp);
	//LOG("connect ok!");
	
	string s;
	makePostRequestHead(s, url, pInputHeader, data);
	
	vector<uint8_t> senddata;
	for(size_t i = 0; i < s.size(); ++i)
	{
		senddata.push_back(s[i]);
	}

	for(size_t i = 0; i < data.size(); ++i)
	{
		senddata.push_back(data[i]);
	}
	
	//cout << "===================================" << endl;
	//cout << senddata << endl;
    //cout << "===================================" << endl;

	unsigned int numbytes = 0U;
	while(numbytes < senddata.size())
	{
	    int rv = send(m_sockfd, &senddata[0] + numbytes, senddata.size() - numbytes, 0);
	    if(rv < 0)
	    {
			LOG("send error");
			close(m_sockfd);
			return -2;
	    }
		
		numbytes += rv;
		//LOG(lexical_cast<string>(numbytes) + "bytes send");
	}
	
	LOG("send seccess!");
	
	vector<uint8_t> cvecWithHead;
	int pos = recvHttpData2(cvecWithHead, timeout);
	if(pos < 0)
    {
        cout << "recvHttpData error: " << pos << endl;
        close(m_sockfd);
        return -3;
    }
	if(cvecWithHead.empty() == true)
	{
		close(m_sockfd);
		return -4;
	}
	
	if(pheader != NULL)
	{
		const string sHead(&cvecWithHead[0], &cvecWithHead[0] + pos);
		
		//cout << sHead << endl;
		
		makeRespHead(sHead, *pheader);
	}
	
	cvec.assign(&cvecWithHead[0] + pos, &cvecWithHead[0] + cvecWithHead.size());
	if(cvec.size() >= 3)
    {
	    string sFirst3chars(cvec.begin(), cvec.begin() + 3);
	    if(sFirst3chars == "\x1F\x8B\x08")
	    {
	    	vector<uint8_t> unzipedData;
	    	
	    	// ungzip
	    	ungzipxp(&cvec[0], cvec.size(), unzipedData);
	    	cvec.swap(unzipedData);;
	    }
    }
	
	close(m_sockfd);
	return 0;	
}

HttpHeaderItem HttpClient::divideArg(const string &sLine, const string &sDelem) const
{
	string::size_type pos = sLine.find(sDelem);
	if(pos != string::npos)
	{
		string sKey(sLine.substr(0, pos));
		string sValue(sLine.substr(pos + sDelem.size()));
		if(sValue.empty() == false && sValue[0] == ' ')
		{
			sValue.erase(0, 1);
		}
		
	    return HttpHeaderItem(sKey, sValue);
	}
	else
	{
		return HttpHeaderItem("", "");
	}
}

bool HttpClient::makeRespHead(const string &sHead, HttpHeader &header) const
{
	header.clear();
	
	// status
	string sStatusSec;
	string::size_type pos = Substr(sStatusSec, sHead, "HTTP", "\r\n");
	if(pos == string::npos)
	{
		return false;
	}
	
	LOG("sStatusSec = " + sStatusSec);
	
	string sStatus;
	if(Substr(sStatus, sStatusSec, " ", " ") == string::npos)
	{
		return false;
	}
	
	LOG("sStatus = " + sStatus);
	
	if(isStringDigit(sStatus) == false)
	{
		return false;
	}
	
	header.m_HttpheaderVec.push_back(HttpHeaderItem("Status", sStatus));
	
	// 循环解析
	while(true)
	{
		pos -= 2;
		
		string sLine;
		pos = Substr(sLine, sHead, "\r\n", "\r\n", pos);
		if(pos == string::npos || sLine.empty() == true)
		{
			break;
		}
		
		HttpHeaderItem hhi = divideArg(sLine, ":");
		header.m_HttpheaderVec.push_back(std::move(hhi));
	}
	
	return true;
}

void HttpClient::makeGetRequestHead(string &s0, const URL &url, const HttpHeader *phead, const int beg_pos, const int end_pos) const
{
	s0 = "GET " + url.path() + " HTTP/1.1\r\n";
	if(beg_pos >= 0)
	{
		if(end_pos < 0)
		{
			// 断点续传(无末尾)
			// "Range: bytes=0-\r\n"
			string s1 = string("Range: bytes=") + lexical_cast<string>(beg_pos) + "-\r\n";
			s0 += s1;
		}
		else
		{
			// 断点续传
			// "Range: bytes=0-100\r\n"
			string s1 = string("Range: bytes=") + lexical_cast<string>(beg_pos) + "-" + lexical_cast<string>(beg_pos) + "\r\n";
			s0 += s1;
		}
	}
	
	s0 += "Host: " + url.host() + "\r\n";

	if(phead != NULL)
	{
		for(unsigned int i = 0; i < phead->m_HttpheaderVec.size(); ++i)
		{
			string s1 = phead->m_HttpheaderVec[i].m_sKey + ": " + phead->m_HttpheaderVec[i].m_sValue + "\r\n";
			s0.append(s1);
		}
	}
    s0 += "\r\n";
}

void HttpClient::makePostRequestHead(string &s0, const URL &url, const HttpHeader *phead, const vector<uint8_t> &data) const
{
	s0 = "POST " + url.path() + " HTTP/1.1\r\n";
	s0 += "Content-Length: " + lexical_cast<string>(data.size()) + "\r\n";
	s0 += "Host: " + url.host() + "\r\n";
	
	if(phead != NULL)
	{
		for(unsigned int i = 0; i < phead->m_HttpheaderVec.size(); ++i)
		{
			s0 += phead->m_HttpheaderVec[i].m_sKey + ": " + phead->m_HttpheaderVec[i].m_sValue + "\r\n";
		}
	}
	
    s0 += "\r\n";
}

int HttpClient::gzcompress(const unsigned char *data, unsigned long ndata, unsigned char *zdata, unsigned long *nzdata) const
{  
	z_stream c_stream;  
	int err = 0;  

	if(data && ndata > 0)
	{  
		c_stream.zalloc = NULL;  
		c_stream.zfree = NULL;  
		c_stream.opaque = NULL;  
		
		// 只有设置为MAX_WBITS + 16才能在在压缩文本中带header和trailer  
		if(deflateInit2(&c_stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, MAX_WBITS + 16, 8, Z_DEFAULT_STRATEGY) != Z_OK)
		{
			return -1;
		}

		c_stream.next_in = (unsigned char*)data;
		c_stream.avail_in = ndata;  
		c_stream.next_out = zdata;  
		c_stream.avail_out = *nzdata;  
		while(c_stream.avail_in != 0 && c_stream.total_out < *nzdata)
		{  
			if(deflate(&c_stream, Z_NO_FLUSH) != Z_OK)
			{
				return -1;
			}
		}

		if(c_stream.avail_in != 0)
		{
			return c_stream.avail_in;
		}

		while(true)
		{  
			if((err = deflate(&c_stream, Z_FINISH)) == Z_STREAM_END)
			{
				break;
			}

			if(err != Z_OK)
			{
				return -1;
			}
		}

		if(deflateEnd(&c_stream) != Z_OK)
		{
			return -1;
		}

		*nzdata = c_stream.total_out;  
		return 0;  
	}

	return -1;  
}  

int HttpClient::ungzipxp(const uint8_t *source, const int len, vector<uint8_t> &desvec) const
{
    desvec.clear();
       
    const unsigned long int uncomprLen = len * 4;
       
	Byte *uncompr = new Byte[uncomprLen];
	memset(uncompr, 0, uncomprLen);

	//但是gzip或者zlib数据里\0很多。
	
	strcpy((char*)uncompr, "garbage");

    z_stream d_stream;
	d_stream.zalloc = Z_NULL;
	d_stream.zfree = Z_NULL;
	d_stream.opaque = Z_NULL;
	d_stream.next_in = Z_NULL;//inflateInit和inflateInit2都必须初始化next_in和avail_in
	d_stream.avail_in = 0;//deflateInit和deflateInit2则不用

	int ret = inflateInit2(&d_stream, 47);
	if(ret != Z_OK)
	{
		printf("inflateInit2 error: %d",ret);
		return ret;
	}
	
    d_stream.next_in = (unsigned char *)source;
	d_stream.avail_in = len;
	
    do
	{
		d_stream.next_out = uncompr;
		d_stream.avail_out = uncomprLen;
		
		ret = inflate(&d_stream, Z_NO_FLUSH);
		if(ret == Z_STREAM_ERROR)
		{
             return -99;
        }
        
		switch(ret)
		{
    		case Z_NEED_DICT:
    			ret = Z_DATA_ERROR;   
    		case Z_DATA_ERROR:
    		case Z_MEM_ERROR:
    			inflateEnd(&d_stream);
    			return ret;
		}
		int have = uncomprLen - d_stream.avail_out;
		
		for(int i = 0; i < have; ++i)
        {
		    desvec.push_back(uncompr[i]);
		}
	}
    while(d_stream.avail_out == 0);
	inflateEnd(&d_stream);
	delete []uncompr;
	return ret;
}

string domainToIp(const string &sDomain)
{
	if(isIpType(sDomain.c_str()) == false)
	{
		cout << "is not ip" << endl;
		hostent *site = gethostbyname(sDomain.c_str());
		if(site == NULL)
		{
			cout << "gethostbyname error!" << endl;
			return "";
		}
		
		for(char **p = site->h_addr_list; *p != NULL; ++p)
		{
			return inet_ntoa(*(in_addr *)(*p));
		}
	}
	else
	{
		cout << "is ip" << endl;
		return sDomain;
	}
	
	return "";
}
