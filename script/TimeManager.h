#ifndef _TIME_MANAGER_H_
#define _TIME_MANAGER_H_

#include "typedefine.h"

class TimeManager
{
public:
    TimeManager(const time_t second = time(NULL));
    
    void loadFromSecond(const time_t second);
    void loadFromString(const string &s, const char *format);
    
    string toString(const char *format);
    time_t toTimeT();
    
    int getDayOfWeek() const;
    
    int getHour() const;
    int getMinite() const;
    int getSecond() const;
    
    string toHttpString();
    
private:
    tm m_tm;
};

#endif

