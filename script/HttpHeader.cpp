#include "HttpHeader.h"

/*-----------------------------------------------------------------------*/
HttpHeaderItem::HttpHeaderItem()
{
	
}

HttpHeaderItem::HttpHeaderItem(const string &sKey, const string &sValue)
    : m_sKey(sKey), m_sValue(sValue)
{
}

HttpHeaderItem::HttpHeaderItem(string &&sKey, string &&sValue)
    : m_sKey(std::move(sKey)), m_sValue(std::move(sValue))
{
}


void HttpHeaderItem::load(const string &sKey, const string &sValue)
{
	m_sKey = sKey;
	m_sValue = sValue;
}

void HttpHeaderItem::emplace(string &&sKey, string &&sValue)
{
	m_sKey.swap(sKey);
	m_sValue.swap(sValue);
}

void HttpHeaderItem::clear()
{
	m_sKey.clear();
	m_sValue.clear();
}

void HttpHeaderItem::show(ostream &os) const
{
	cout << m_sKey << ": " << m_sValue << endl;
}
/*-----------------------------------------------------------------------*/
void HttpHeader::clear()
{
	m_HttpheaderVec.clear();
}

void HttpHeader::show(ostream &os) const
{
    TraceLog log("HttpHeader");
	for(unsigned int i = 0; i < m_HttpheaderVec.size(); ++i)
	{
		m_HttpheaderVec[i].show(os);
	}
}

bool HttpHeader::empty() const
{
    return m_HttpheaderVec.empty();
}

string HttpHeader::operator [](const string &sKey) const
{
	for(unsigned int i = 0; i < m_HttpheaderVec.size(); ++i)
	{
		if(m_HttpheaderVec[i].m_sKey == sKey)
		{
			return m_HttpheaderVec[i].m_sValue;
		}
	}
	
	return "";
}

void HttpHeader::insert(const string &sKey, const string &sValue)
{
	m_HttpheaderVec.push_back(HttpHeaderItem());
	m_HttpheaderVec.back().load(sKey, sValue);
}

void HttpHeader::emplace(string &&sKey, string &&sValue)
{
	m_HttpheaderVec.push_back(HttpHeaderItem(std::move(sKey), std::move(sValue)));
}

void HttpHeader::getCookie(string &orgCookie) const
{
	string sCookie;
	for(unsigned int i = 0; i < m_HttpheaderVec.size(); ++i)
	{
		if(m_HttpheaderVec[i].m_sKey == "Set-Cookie")
		{
			const string &sValue = m_HttpheaderVec[i].m_sValue;
			string::size_type pos = sValue.find(";");
			if(pos != string::npos)
			{
				string sACookie = sValue.substr(0, pos);
				
				if(sCookie.empty() == false)
				{
					sCookie += "; ";
				}
				
				sCookie += sACookie;
			}
			else
			{
				if(!sValue.empty())
				{
					if(sCookie.empty() == false)
					{
						sCookie += "; ";
					}
					
					sCookie += sValue;
				}
			}
		}
	}
	
	if(!orgCookie.empty())
	{
		if(!sCookie.empty())
		{
			orgCookie = orgCookie + "; " + sCookie;
		}
	}
	else
	{
		if(!sCookie.empty())
		{
			orgCookie.swap(sCookie);
		}
	}

}
