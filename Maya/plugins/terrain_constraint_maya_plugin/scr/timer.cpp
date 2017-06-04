/**
 * Simple timer, linux only
 */
#ifdef linux
#include <sys/time.h>

#include "../include/timer.h"
#include "utils.h"

using namespace std;

Timer::Timer(bool a_Start)
{
   if (a_Start)
   {
      Start();
   }
}

Timer::~Timer()
{
}

void Timer::Start()
{
   gettimeofday(&m_TimeVal, &m_TimeZone);
}

float Timer::Elapsed() const
{
   struct timeval tv;
   struct timezone tz;
   gettimeofday(&tv, &tz);
   float now = (tv.tv_sec - m_TimeVal.tv_sec)
   + 0.000001f * (tv.tv_usec - m_TimeVal.tv_usec);
   return now;
}

XTimer::XTimer() :
Timer(false), m_Elapsed(0.0f), m_Count(0)
{
}

void XTimer::Stop()
{
   m_Elapsed += Elapsed();
}

void XTimer::Reset()
{
   m_Elapsed = 0.0f;
   m_Count = 0;
}

void TimerManager::Reset()
{
   for (map<string, XTimer>::iterator it = m_XTimers.begin();
         it != m_XTimers.end(); ++it)
   {
      XTimer & timer = (*it).second;
      timer.Reset();
   }
}

XTimer & TimerManager::GetTimer(const char * a_File, int a_Line,
      const char * a_Name)
{
   return m_XTimers[a_Name];
}

std::string TimerManager::GetLevelString() const
{
   string r;
   for (int i = 0; i < m_Level; ++i)
   r += "    ";
   return r;
}
#endif//linux
