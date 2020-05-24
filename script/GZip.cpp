#include "GZip.h"

inline int gzcompress(const unsigned char* data, unsigned long ndata, unsigned char* zdata, unsigned long* nzdata)
{
	if (data == NULL || ndata <= 0)
	{
		return -1;
	}

	int err = 0;

	z_stream c_stream;
	c_stream.zalloc = NULL;
	c_stream.zfree = NULL;
	c_stream.opaque = NULL;

	// 只有设置为MAX_WBITS + 16才能在在压缩文本中带header和trailer  
	if (deflateInit2(&c_stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, MAX_WBITS + 16, 8, Z_DEFAULT_STRATEGY) != Z_OK)
	{
		return -1;
	}

	c_stream.next_in = (unsigned char*)data;
	c_stream.avail_in = ndata;
	c_stream.next_out = zdata;
	c_stream.avail_out = *nzdata;
	while (c_stream.avail_in != 0 && c_stream.total_out < *nzdata)
	{
		if (deflate(&c_stream, Z_NO_FLUSH) != Z_OK)
		{
			deflateEnd(&c_stream);
			return -2;
		}
	}

	if (c_stream.avail_in != 0)
	{
		deflateEnd(&c_stream);
		return c_stream.avail_in;
	}

	while (true)
	{
		if ((err = deflate(&c_stream, Z_FINISH)) == Z_STREAM_END)
		{
			break;
		}

		if (err != Z_OK)
		{
			deflateEnd(&c_stream);
			return -3;
		}
	}

	deflateEnd(&c_stream);

	*nzdata = c_stream.total_out;
	return 0;
}
inline int ungzipxp(const uint8_t* source, const int len, vector<uint8_t>& desvec)
{
	desvec.clear();

	const unsigned long int uncomprLen = len * 4;

	vector<uint8_t> vec;
	vec.resize(uncomprLen);
	uint8_t* uncompr = &vec[0];

	//但是gzip或者zlib数据里\0很多。

	strcpy((char*)uncompr, "garbage");

	z_stream d_stream;
	d_stream.zalloc = Z_NULL;
	d_stream.zfree = Z_NULL;
	d_stream.opaque = Z_NULL;
	d_stream.next_in = Z_NULL;//inflateInit和inflateInit2都必须初始化next_in和avail_in
	d_stream.avail_in = 0;//deflateInit和deflateInit2则不用

	int ret = inflateInit2(&d_stream, 47);
	if (ret != Z_OK)
	{
		printf("inflateInit2 error: %d", ret);
		inflateEnd(&d_stream);
		return ret;
	}

	d_stream.next_in = (unsigned char*)source;
	d_stream.avail_in = len;

	do
	{
		d_stream.next_out = uncompr;
		d_stream.avail_out = uncomprLen;

		ret = inflate(&d_stream, Z_NO_FLUSH);
		if (ret == Z_STREAM_ERROR)
		{
			break;
		}

		switch (ret)
		{
		case Z_NEED_DICT:
			ret = Z_DATA_ERROR;
		case Z_DATA_ERROR:
		case Z_MEM_ERROR:
			inflateEnd(&d_stream);
			return ret;
		}
		int have = uncomprLen - d_stream.avail_out;

		for (int i = 0; i < have; ++i)
		{
			desvec.push_back(uncompr[i]);
		}
	} while (d_stream.avail_out == 0);
	inflateEnd(&d_stream);
	return ret;
}
