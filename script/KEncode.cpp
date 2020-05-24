#include "KEncode.h"

KEncode::KEncode(const int ixor)
    : m_xor(ixor)
{
}

bool KEncode::Encode(const ContainerType &vIn, ContainerType &vOut, const int inum) const
{
	vOut.clear();
	if(vIn.empty() == true)
	{
	    return true;	
	}
	
	const size_t size = vIn.size();
	
	// 右移位数因子对照表
	static const byte ARR[9] = {0, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff};
	const int num = inum % 8;  //0-7的整形

	if(num < 0 || num > 8)
	{
		//AfxMessageBox(_T("右移长度错误，最多8位"));		
		return false;
	}
    
    vOut.clear();
	copy(vIn.begin(), vIn.end(), back_inserter(vOut));

	byte *pData = &vOut[0];
	byte a = 0;
	byte b = 0;

	int ixor = m_xor;
	for(size_t i = 0; i < size; ++i)
	{
		a = pData[i] & ARR[num];
		pData[i] >>= num;
		b = a << (8 - num);
		pData[i] |= b;
		ixor = abs((ixor + 153) * (ixor - 78) + 3) % 256;
		pData[i] ^= ixor;
	}
	
	return true;
}

bool KEncode::Decode(const ContainerType &vIn, ContainerType &vOut, const int inum) const
{
	vOut.clear();
	
	if(vIn.empty() == true)
	{
	    return true;	
	}
	
	const size_t size = vIn.size();
	
	// 右移位数因子对照表
	static const byte ARR[9] = {0, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff};
	const int num = inum % 8;  //0-7的整形
	if(num < 0 || num > 8)
	{
		//AfxMessageBox(_T("右移长度错误，最多8位"));		
		return false;
	}

    vOut.clear();
	copy(vIn.begin(), vIn.end(), back_inserter(vOut));

	byte *pData = &vOut[0];
	byte a = 0;
	byte b = 0;
	int ixor = m_xor;
	for(size_t i = 0; i < size; ++i)
	{		
		ixor = abs((ixor + 153) * (ixor - 78) + 3) % 256;
		pData[i] ^= ixor;
		a = pData[i] & ARR[num];
		pData[i] >>= num;
		b = a << (8 - num);
		pData[i] |= b;
	}
	
	return true;
}

bool KEncode::EncodeToChar(const ContainerType &vIn, string &sOut, const int inum) const
{
	sOut.clear();
	vector<uint8_t> cvec;
	bool rb = Encode(vIn, cvec, inum);
	HexToChar(sOut, cvec);
	return rb;
}

bool KEncode::DecodeFromChar(const string &sIn, ContainerType &vOut, const int inum) const
{
	vOut.clear();
	vector<uint8_t> cvec;
	CharToHex(cvec, sIn);
	bool rb = Decode(cvec, vOut, inum);
	return rb;
}

string &KEncode::HexToChar(string &s, const vector<uint8_t> &data) const
{
	s.clear();
	for(unsigned int i = 0; i < data.size(); ++i)
	{
	    char szBuff[3] = "";
	    sprintf(szBuff, "%02x", *reinterpret_cast<const unsigned char *>(&data[i]));
	    s.push_back(szBuff[0]);
	    s.push_back(szBuff[1]);
	}
	return s;
}

void KEncode::CharToHex(vector<uint8_t> &data, const string &s) const
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
