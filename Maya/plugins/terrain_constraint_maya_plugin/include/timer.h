#ifndef util_Timer_H
#define util_Timer_H

#ifdef linux
#include <sys/time.h>

#include <map>
#include <string>
#include "include/singleton.h"
using namespace std;
class Timer
{

   public:
   Timer(bool a_Start = true);
   ~Timer();

   void Start();
   float Elapsed() const;

   private:
   struct timeval m_TimeVal;
   struct timezone m_TimeZone;
};

class XTimer: public Timer
{

   public:
   XTimer();

   void Stop();
   void Reset();
   float GetTotalElapsed() const
   {
      return m_Elapsed;
   }

   inline void IncCount()
   {
      m_Count++;
   }
   inline int GetCount() const
   {
      return m_Count;
   }

   private:
   float m_Elapsed;
   int m_Count;
   struct timeval m_TimerVal;
   struct timezone m_TimeZone;
};

class TimerManager: public Singleton<TimerManager>
{

   public:
   TimerManager() :
   m_Level(0)
   {
   }
   void Reset();
   XTimer & GetTimer(const char * a_File, int a_Line, const char * a_Name);

   void IncLevel()
   {
      ++m_Level;
   }
   void DecLevel()
   {
      --m_Level;
   }
   int GetLevel() const
   {
      return m_Level;
   }
   string GetLevelString() const;

   const map<string, XTimer> & GetXTimers() const
   {
      return m_XTimers;
   }

   private:
   map<string, XTimer> m_XTimers;
   int m_Level;
};

//#define ENABLE_TIMINGS
#ifdef ENABLE_TIMINGS
#define _GET_TIMER(X) static XTimer & X##_Timer = TimerManager::Instance().GetTimer( __FILE__, __LINE__, #X )
#define TIMER_START(X) if (true) { _GET_TIMER(X); X##_Timer.Start(); TimerManager::Instance().IncLevel(); LOG(INFO, ("%s >>> %s", TimerManager::Instance().GetLevelString().c_str(), #X ) ); } else {}
#define TIMER_INC_COUNT(X) if (true) { _GET_TIMER(X); X##_Timer.IncCount(); LOG(INFO, ("%s   + %s", TimerManager::Instance().GetLevelString().c_str(), #X ) ); } else {}
#define TIMER_STOP(X) if (true) { _GET_TIMER(X); X##_Timer.Stop(); LOG(INFO, ("%s <<< %s", TimerManager::Instance().GetLevelString().c_str(), #X ) ); TimerManager::Instance().DecLevel(); } else {}
#else
#define TIMER_START(X)
#define TIMER_INC_COUNT(X)
#define TIMER_STOP(X)
#endif

#endif
#endif//linux
