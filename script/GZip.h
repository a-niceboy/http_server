#ifndef _GZIP_H_
#define _GZIP_H_

inline int gzcompress(const unsigned char* data, unsigned long ndata, unsigned char* zdata, unsigned long* nzdata);
/*
	string str1 = "abcdefg";
	
	vector<uint8_t> vec;
	vec.resize(str1.size() + 1000);
	long unsigned int len = vec.size();
	gzcompress((unsigned char*)str1.c_str(), str1.size(), &vec[0], &len);
	vec.resize(len);

	string str2(vec.begin(), vec.end());
*/

inline int ungzipxp(const uint8_t* source, const int len, vector<uint8_t>& desvec);



#endif