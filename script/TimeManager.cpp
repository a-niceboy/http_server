#include "TimeManager.h"

TimeManager::TimeManager(const time_t second)
{
    // ×ª»»GTM
   loadFromSecond(second);
}

string TimeManager::toHttpString()
{
    //"Fri, 09 Jul 2010 09:14:09 GMT"   
    return toString("%a, %d %b %Y %H:%M:%S GTM");
}

void TimeManager::loadFromSecond(const time_t second)
{
    // ×ª»»GTM
    localtime_r(&second, &m_tm);
}

void TimeManager::loadFromString(const string &s, const char *format)
{
    strptime(s.c_str(), format, &m_tm);
}

string TimeManager::toString(const char *format)
{
    char szTime[50] = "";
    strftime(szTime, 50, format, &m_tm);
    
    return szTime;       
}

time_t TimeManager::toTimeT()
{
    return mktime(&m_tm);       
}

int TimeManager::getDayOfWeek() const
{
	return m_tm.tm_wday;	/* Days since Sunday (0-6) */
}

int TimeManager::getHour() const
{
	return m_tm.tm_hour;
}

int TimeManager::getMinite() const
{
	return m_tm.tm_min;
}

int TimeManager::getSecond() const
{
	return m_tm.tm_sec;
}

