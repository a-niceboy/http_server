#ifndef _KENCODE_H_
#define _KENCODE_H_

#include "typedefine.h"

typedef unsigned char byte;
typedef vector<uint8_t> ContainerType;

class KEncode
{
public:
	KEncode(const int ixor = 28);

	bool Encode(const ContainerType &vIn, ContainerType &vOut, const int inum = 7) const;
	bool Decode(const ContainerType &vIn, ContainerType &vOut, const int inum = 1) const;
	
	bool EncodeToChar(const ContainerType &vIn, string &sOut, const int inum = 7) const;
	bool DecodeFromChar(const string &sIn, ContainerType &vOut, const int inum = 1) const;
	
	string &HexToChar(string &s, const vector<uint8_t> &data) const;
	void CharToHex(vector<uint8_t> &data, const string &s) const;
	
public:
	const int m_xor;
};

#endif


