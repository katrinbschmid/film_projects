/**
 * helper
 */

#include "../include/singleton.h"

#define SINGLETON_INSTANCE(CLASS) template<> CLASS * Singleton< CLASS >::m_Instance = NULL

class TimerManager;
SINGLETON_INSTANCE(TimerManager);
