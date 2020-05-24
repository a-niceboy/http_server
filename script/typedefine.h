#ifndef _TYPEDEFINE_H_
#define _TYPEDEFINE_H_

// 标准c++头文件
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <queue>
#include <set>
#include <map>
#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <ctime>
using namespace std;

// linux系统头文件
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <netdb.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <ifaddrs.h>
#include <sys/file.h>
#include <zlib.h>
#include <uuid/uuid.h>
#include <sys/ioctl.h>
#include <poll.h>


#include <json/json.h>
using namespace Json;

#define VHEAD "/root/imsg/imsgClientServer"
#define PORT 6123

extern string G_HOST;
extern string DB_LOGIN_STRING;

typedef vector<string> ContainerType1;
typedef vector<uint8_t> ByteArray;
typedef map<string, string> URLContainer;

#define VPATH "VPath"

class HttpServer;
class InterMediary
{
public:
    InterMediary()
    {
    	pServer = NULL;
    }
    
public:
	HttpServer *pServer;
};

extern InterMediary g_IM;

typedef int (*PFI1)(int ch);

class TraceLog
{
public:
	TraceLog(const string &s, ostream &os = cout)
	    : m_os(os), m_s(s)
	{
		m_ltime = time(NULL);
		os << "--------" << s << "-------->" << endl;
	}
		
	~TraceLog()
	{
		time_t m_ltime1 = time(NULL);
		m_os << "<--------" << m_s << "----" << m_ltime1 - m_ltime << "----" << endl;
	}
	
private:
	ostream &m_os;
	string m_s;
	time_t m_ltime;
};

class TraceLog3
{
public:
	TraceLog3(const string &s, ostream &os = cout)
	    : m_os(os), m_s(s)
	{
		gettimeofday(&m_tv, NULL);
		os << "--------" << s << "-------->" << endl;
	}
		
	~TraceLog3()
	{
		timeval tv2;
		gettimeofday(&tv2, NULL);
		m_os << "<--------" << m_s << "----" << tv2.tv_sec - m_tv.tv_sec << '.' << tv2.tv_usec - m_tv.tv_usec << "----" << endl;
	}
	
private:
	ostream &m_os;
	string m_s;
	timeval m_tv;
};


/**
 *    不同类型的数据转换成字符串的函数
 */
template<typename T1, typename T2>
inline T1 lexical_cast(const T2 &t)
{
    stringstream ss;
    
    ss << t;
    T1 tReturn;
    
    ss >> tReturn;
    
    return tReturn;
}

// 字符串截取
// 输出参数，输入参数，头，尾
inline string::size_type Substr(string &sSub, const string &s, const string &sHead, const string &sTail, const string::size_type pos = 0)
{
	if(pos >= s.size())
	{
		return string::npos;
	}
	
    string::size_type pos1 = 0;
    string::size_type pos2 = 0;
    
	if((pos1 = s.find(sHead, pos)) == string::npos)
	{
		return string::npos;
	}   
	
	if((pos2 = s.find(sTail, pos1 + sHead.size())) == string::npos)
	{
		string sErr = "cant find word: " + sTail;
		return string::npos;
	}
	
	sSub = s.substr(pos1 + sHead.size(), pos2 - pos1 - sHead.size());
	
	return pos2 + sTail.size();
}

inline void StringToLower(string &s)
{
	for(size_t i = 0; i < s.size(); ++i)
	{
	    if(isupper(s[i]) != 0)
	    {
	    	s[i] = tolower(s[i]);
	    }	
	}
}

inline void getUrlFileName(string &sFileName, const string &sUrl)
{
	string::size_type pos = sUrl.rfind("/");
	if(pos != string::npos)
	{
	    sFileName.assign(sUrl.substr(pos + 1));
	}
}

inline string::size_type SubstrF(string &sSub, const string &s, const string &sHead, const string &sTail, const string::size_type pos = 0)
{
	if(pos >= s.size())
	{
		return string::npos;
	}
	
	string s1(s);
	StringToLower(s1);
	
	string sHead1(sHead);
	StringToLower(sHead1);
	
	string sTail1(sTail);
	StringToLower(sTail1);
	
    string::size_type pos1 = 0;
    string::size_type pos2 = 0;
    
	if((pos1 = s1.find(sHead1, pos)) == string::npos)
	{
		return string::npos;
	}   
	
	if((pos2 = s1.find(sTail1, pos1 + sHead1.size())) == string::npos)
	{
		string sErr = "cant find word: " + sTail;
		return string::npos;
	}
	
	sSub = s.substr(pos1 + sHead.size(), pos2 - pos1 - sHead.size());	
	return pos2 + sTail.size();
}

// 读取文件的函数
inline bool ReadFile(string &s, const string &sPath)
{
	// 判断文件是否可读
    struct stat st;
    if(lstat(sPath.c_str(), &st) < 0)
    {
    	// pathname是文件名的绝对路径
        printf("lstate error %d\n", errno);
        return -errno;
    }
    
    if(S_ISDIR(st.st_mode) != 0)
    {
    	// 是文件夹
    	cout << "is dir!" << endl;
        return false;
    }
    
    const int size = st.st_size;
	
    ifstream in(sPath.c_str(), ios::binary);

    if(!in)
    {
    	string sErr = "con't open file " + sPath;
    	cout << sErr << endl;
        return false;
    }

    s.resize(size);
    in.read(&s[0], size);

    return true;
}

// 读取文件的函数
inline bool ReadFile(vector<uint8_t> &cvec, const string &sPath)
{
	// 判断文件是否可读
    struct stat st;
    if(lstat(sPath.c_str(), &st) < 0)
    {
    	// pathname是文件名的绝对路径
        printf("lstate error %d\n", errno);
        return -errno;
    }
    
    if(S_ISDIR(st.st_mode) != 0)
    {
    	// 是文件夹
    	cout << "is dir!" << endl;
        return false;
    }
    
    const int size = st.st_size;

    ifstream in(sPath.c_str(), ios::binary);

    if(!in)
    {
    	string sErr = "con't open file " + sPath;
    	cout << sErr << endl;
        return false;
    }

    cvec.resize(size);
    in.read((char*)&cvec[0], size);

    return true;
}

inline void writeFile(const string &s, const string &sFileName) throw(logic_error)
{
    ofstream out(sFileName.c_str(), ios::binary);
    if(!out)
    {
        throw logic_error("无法打开输出文件！");
    }
    
    copy(s.begin(), s.end(), ostreambuf_iterator<char>(out));
}


/**
 *   保存文件函数， 输入参数为要保存的数据
 */
inline void save(const char *buff, const string &sFileName) throw(logic_error)
{
    ofstream out(sFileName.c_str(), ios::binary);
    if(!out)
    {
        throw logic_error("无法打开输出文件！");
    }
    out << buff;
    
    cout << "保存路径是： " << sFileName << endl;
    cout << "保存文件成功！" << endl;
}


inline int findSz(const char *sz1, const char *sz2, int size1, int size2)
{
	if(sz1 == NULL || sz2 == NULL)
	{
	    return -1;	
	}
	
    if(size1 < size2)
    {
        return -1;         
    }
    
	for(int i = 0; i < size1 - size2 + 1; ++i)
	{
		for(int j = 0; j < size2; ++j)
		{
			if(sz1[i + j] != sz2[j])
			{
				break;
			}
			
			if(j == size2 - 1)
			{
                return i;    
            }
		}
	}
	
	return -1;
}

inline bool isStringRight(const string &s, PFI1 fun)
{
	const int size = s.size();
    for(int i = 0; i < size; ++i)
    {
        if(fun(s[i]) == 0)
        {
            return false;	
        }	
    }
    
    return true;
}

class Deleter
{
public:
	template <typename T>
	void operator ()(T *p)
	{
	    delete p;	
	}
};

inline bool isStringDigit(const string &s)
{
	if(s.size() == 0)
	{
	    return false;	
	}
	
    for(size_t i = 0; i < s.size(); ++i)
    {
    	if(isdigit(s[i]) == 0)
    	{
    		return false;
    	}
    }
    
    return true;
}

inline void AddLengthAndString(string &sdes, const size_t numLength, const string &s)
{
	string sLen = lexical_cast<string>(s.size());
	while(sLen.size() < numLength)
	{
		sLen.insert(0, 1, '0');
	}
	
	sdes = sLen + s;
}

inline const char *parserAValue(const char *cur_pos, const int len, string &s)
{
	string sLength(cur_pos, cur_pos + len);
	if(isStringDigit(sLength) == false)
	{
		return NULL;
	}
	
	cur_pos += len;
	int iLength = atoi(sLength.c_str());
	
	s.assign(cur_pos, cur_pos + iLength);
	cur_pos += iLength;
	return cur_pos;
}

inline bool isIpType(const char *szIp)
{
	const int size = strlen(szIp);
    for(int i = 0; i < size; ++i)
    {
    	// not digit and not .
        if(isdigit(szIp[i]) == 0 && szIp[i] != '.')
        {
            return false;
        }
    }
    
    return true;
}

template <typename T1, typename T2>
inline T2 getMapValue(const map<T1, T2> &mp, const T1 &Key)
{
	URLContainer::const_iterator citer = mp.find(Key);
	if(citer != mp.end())
	{
		return citer->second;
	}
	else
	{
		return T2();
	}
}

template <typename T1, typename T2>
inline T2 getFirstValue(const multimap<T1, T2> &mp, const T1 &Key)
{
	typename multimap<T1, T2>::const_iterator citer = mp.find(Key);
	if(citer != mp.end())
	{
		return citer->second;
	}
	else
	{
		return T2();
	}
}

inline void addUrlKeyValue(string &sdes, const string &sKey, const string &sValue)
{
	sdes = sKey + "=" + sValue;
}

template <typename T1, typename T2>
inline ostream &operator <<(ostream &os, const map<T1, T2> &mp)
{
	for(typename map<T1, T2>::const_iterator iter = mp.begin(); iter != mp.end(); ++iter)
	{
	    os << iter->first << " " << iter->second << endl;	
	}
	
	return os;
}

inline ostream &operator <<(ostream &os, const vector<uint8_t> &cevc)
{
    copy(cevc.begin(), cevc.end(), ostreambuf_iterator<char>(os));
	return os;
}

inline bool sleepx(const long int second, const long int msecond = 0L)
{
	timeval tv;
	tv.tv_sec = second;
	tv.tv_usec = msecond;

	int retval = select(0, NULL, NULL, NULL, &tv);

    if(retval == 0)
    {
    	return true;
    }
    else
    {
    	return false;
    }
}

inline void trim(string &s)
{
    if(!s.empty())
    {
        s.erase(0, s.find_first_not_of(" "));
        s.erase(s.find_last_not_of(" ") + 1);
    }
}


inline void SplitStringtoVectorString(const string &data, const string &split, vector<string> &svec_out)
{
    string::size_type pos = data.find(split);
    string stemp = data;
    while(true)
    {
        if(pos != string::npos)
        {
            svec_out.push_back((stemp.substr(0, pos)));
            stemp = stemp.substr(pos + split.size());
            pos = stemp.find(split);
        }
        else
        {
            svec_out.push_back(stemp);
            break;
        }

    }
}


inline const string getHostIp(const string &sKey = "")
{
    ifaddrs *ifAddrStruct = NULL;
    int err = getifaddrs(&ifAddrStruct);
    if(err != 0)
    {
    	cout << "getifaddrs error." << endl;
        return "";	
    }
    
    // free时候使用
    ifaddrs *ifAddrStruct1 = ifAddrStruct;
    
    while(ifAddrStruct != NULL)
    {
    	sockaddr *addr = ifAddrStruct->ifa_addr;
    	if(addr == NULL)
    	{
    		ifAddrStruct = ifAddrStruct->ifa_next;
    	    continue;	
    	}
    	
        if(addr->sa_family != AF_INET) 
    	{
    		ifAddrStruct = ifAddrStruct->ifa_next;
    	    continue;	
    	}
    	
    	// check it is IP4
        // is a valid IP4 Address
        void *tmpAddrPtr = &((struct sockaddr_in *)addr)->sin_addr;
        if(tmpAddrPtr == NULL)
        {
        	ifAddrStruct = ifAddrStruct->ifa_next;
        	continue;
        }
        
        char addressBuffer[INET_ADDRSTRLEN] = "";
        inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
        printf("%s IP Address %s\n", ifAddrStruct->ifa_name, addressBuffer);
        
        if(strcmp(addressBuffer, "127.0.0.1") != 0)
        {
        	if(sKey.empty())
        	{
        		freeifaddrs(ifAddrStruct1);
                return addressBuffer;
            }
            else
            {
            	string sIfaName = ifAddrStruct->ifa_name;
	            if(sIfaName.find(sKey) != string::npos)
	            {
	            	freeifaddrs(ifAddrStruct1);
	            	return addressBuffer;
	            }
            }
        } 

        ifAddrStruct = ifAddrStruct->ifa_next;
    }

    freeifaddrs(ifAddrStruct1);
    return "";
}

inline string urlEncode(const string &sArg)
{
	string sRv;
	for(unsigned int i = 0; i < sArg.size(); ++i)
	{
		char c = sArg[i];
		if(
			(((c >= 'a') && (c <= 'z'))||
			((c >= 'A') && (c <= 'Z'))||
			(c >= '0' && c <= '9'))||
			(c == '.') ||
			(c == '_') ||
			(c == '-')
			)
		{
			sRv.push_back(c);
		}
		else
		{
			char buff[4] = "";
			buff[0] = '%';
			sprintf(buff + 1, "%02x", c & 0xFF);
			sRv += buff;
		}
	}

	return sRv;
}

inline string &urlDecode(string &sRv, const string &sArg)
{
	for(size_t i = 0; i < sArg.size(); )
	{
		char c = sArg[i];
		if(c != '%')
		{
			sRv.push_back(c);
			++i;
		}
		else
		{
			char c1 = sArg[i + 1];
			unsigned int localui1 = 0L;
			if('0' <= c1 && c1 <= '9')
			{
				localui1 = c1 - '0';
			}
			else if('A' <= c1 && c1 <= 'F')
			{
				localui1 = c1 - 'A' + 10;
			}
			else if('a' <= c1 && c1 <= 'f')
			{
				localui1 = c1 - 'a' + 10;
			}

			char c2 = sArg[i + 2];
			unsigned int localui2 = 0L;
			if('0' <= c2 && c2 <= '9')
			{
				localui2 = c2 - '0';
			}
			else if('A' <= c2 && c2 <= 'F')
			{
				localui2 = c2 - 'A' + 10;
			}
			else if('a' <= c2 && c2 <= 'f')
			{
				localui2 = c2 - 'a' + 10;
			}

			unsigned int ui = localui1 * 16 + localui2;
			sRv.push_back(ui);

			i += 3;
		}
	}
	
	return sRv;
}

inline void showHex(const char *p, int size)
{
	for(int i = 0; i < size; ++i)
	{
		printf("%02x", p[i] & 0xff);
	}

	printf("\n");
}

inline void executeCMD(const char *cmd, string &sResult)   
{
    char buf_ps[1024];
    FILE *ptr = popen(cmd, "r");
    if(ptr == NULL)
    {
    	printf("popen %s error\n", cmd);
    	return;
    }
    
    while(!feof(ptr))
    {
    	memset(buf_ps, 0, sizeof(buf_ps));
    	
    	int count = fread(buf_ps, 1024, 1, ptr);
    	if(count != 1)
    	{
    		copy(buf_ps, buf_ps + strlen(buf_ps), back_inserter(sResult));
    	    break;
    	}
    	
    	copy(buf_ps, buf_ps + sizeof(buf_ps), back_inserter(sResult));
    }
	
    pclose(ptr); 
}  

inline void getProcessVec(vector<string> &svec, const string &sProcessName)
{
	string sCommand = "ps -em | grep " + sProcessName;
	
	string sResult;
	executeCMD(sCommand.c_str(), sResult);
	
	cout << "result: " << sResult << endl;
	
	stringstream ss;
	ss << sResult;
	
	string sWord;
	for(int i = 0; ss >> sWord; ++i)
	{
	    if((i + 1) % 4 == 0)
	    {
	        svec.push_back(sWord);
	    }
	}
}

inline void LOG(const string &s)
{
    cout << s << endl;	
}

inline int gzcompress(const unsigned char *data, unsigned long ndata, unsigned char *zdata, unsigned long *nzdata)
{  
	if(data == NULL || ndata <= 0)
	{
		return -1;
	}
	
	int err = 0;  
    
    z_stream c_stream; 
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
			deflateEnd(&c_stream);
			return -2;
		}
	}

	if(c_stream.avail_in != 0)
	{
		deflateEnd(&c_stream);
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
			deflateEnd(&c_stream);
			return -3;
		}
	}

	deflateEnd(&c_stream);

	*nzdata = c_stream.total_out;  
	return 0;
}  
inline int ungzipxp(const uint8_t *source, const int len, vector<uint8_t> &desvec)
{
	desvec.clear();

	const unsigned long int uncomprLen = len * 4;

    vector<uint8_t> vec;
    vec.resize(uncomprLen);
    uint8_t *uncompr = &vec[0];

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
		inflateEnd(&d_stream);
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
			break;
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
	return ret;
}

inline pair<string, string> divideString(const string &sLine, const string &sDelem)
{
	string::size_type pos = sLine.find(sDelem);
	if(pos != string::npos)
	{
		string sKey(sLine.substr(0, pos));
		string sValue(sLine.substr(pos + sDelem.size()));
		
	    return make_pair(sKey, sValue);
	}
	else
	{
		return make_pair("", "");
	}
}

inline long poll_send(int sockfd, const char *buff, size_t length, int timeout)
{
	pollfd pollfd;
	pollfd.fd = sockfd;
	pollfd.events = POLLOUT;
	
    int retval = poll(&pollfd, 1, timeout * 1000);
    if(retval < 0)
    {
    	cout << "poll error" << endl;
   	    return -2;
    }
    else if(retval == 0)
    {
    	cout << "time out" << endl;
        return -3;	
    }	
    
    return send(sockfd, buff, length, 0);
}

inline long poll_recv(int sockfd, char *buff, int maxlength, int timeout)
{
	pollfd pollfd;
	pollfd.fd = sockfd;
	pollfd.events = POLLIN | POLLHUP |POLLPRI;
	
    int retval = poll(&pollfd, 1, timeout * 1000);
    if(retval < 0)
    {
    	cout << "poll error" << endl;
   	    return -2;
    }
    else if(retval == 0)
    {
    	cout << "time out" << endl;
        return -3;	
    }	

    memset(buff, 0, maxlength);
    return recv(sockfd, buff, maxlength, 0);
}

inline string genVid(bool isUpperCase = false)
{
    unsigned char out[16];
    memset(out, 0, sizeof(out));
    uuid_generate_random(out);

	string sRv;
    for(size_t i = 0; i < sizeof(out); ++i)
    {
    	char buff[3] = "";
    	if(isUpperCase)
    	{
            sprintf(buff, "%02X", out[i] & 0xff);
        }
        else
        {
        	sprintf(buff, "%02x", out[i] & 0xff);
        }
        
        sRv.push_back(buff[0]);
        sRv.push_back(buff[1]);
    }

	sRv.insert(20, 1, '-');
	sRv.insert(16, 1, '-');
	sRv.insert(12, 1, '-');
	sRv.insert(8, 1, '-');

	return sRv;
}

inline vector<uint8_t>::const_iterator SubBytes(vector<uint8_t> &sub, const vector<uint8_t> &data, const vector<uint8_t> &head, 
	                                         const vector<uint8_t> &tail, vector<uint8_t>::const_iterator iterBegin)
{
	if(iterBegin >= data.end())
	{
		return data.end();
	}
	
    vector<uint8_t>::const_iterator iter1 = search(iterBegin, data.end(), head.begin(), head.end());
    if(iter1 == data.end())
    {
        cout << "can not find: " << head << endl;
        return data.end();
    }
     
    vector<uint8_t>::const_iterator iter2 = search(iter1 + head.size(), data.end(), tail.begin(), tail.end());
    if(iter2 == data.end())
    {
        cout << "can not find: " << tail << endl;
        return data.end();
    }
    
    sub.assign(iter1 + head.size(), iter2);
	return iter2 + tail.size();
}

inline string &HexToChar(string &s, const vector<uint8_t> &data)
{
    s = "";
    for(unsigned int i = 0; i < data.size(); ++i)
    {
        char szBuff[3] = "";
        sprintf(szBuff, "%02x", *reinterpret_cast<const unsigned char *>(&data[i]) & 0xff);
        s += szBuff[0];
        s += szBuff[1];
    }
    return s;
}

inline void CharToHex(vector<uint8_t> &data, const string &s)
{
    data.clear();
    
    unsigned int ui = 0L;
    for(unsigned int i = 0; i < s.size(); ++i)
    {
        unsigned int localui = 0L;
        const char c = s[i];
        if('0' <= c && c <= '9')
        {
            localui = c - '0';
        }
        else if('A' <= c && c <= 'F')
        {
            localui = c - 'A' + 10;
        }
        else if('a' <= c && c <= 'f')
        {
            localui = c - 'a' + 10;
        }
        
        if(i % 2 == 0)
        {
            ui = localui * 16L;
        }
        else
        {
            ui += localui;
            data.push_back(ui);
        }
    }
}

inline string getSelfName()
{
	pid_t pid = getpid();
    string sPath = "/proc/" + lexical_cast<string>(pid) + "/exe";
	
	char szBuff[1024] = "";
    ssize_t iBytes = readlink(sPath.c_str(), szBuff, sizeof(szBuff));
	if(iBytes < 0)
	{
	    return "";	
	}
	
	string sExeFileName;
	getUrlFileName(sExeFileName, szBuff);
	return sExeFileName;
}

inline string strerror_s(int32_t errnum)
{
	char buf[256] = {0};
	strerror_r(errnum, buf, sizeof(buf) - 1);
	return buf;
}

#endif
