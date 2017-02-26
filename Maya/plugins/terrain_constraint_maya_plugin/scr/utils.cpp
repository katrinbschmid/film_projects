/**
 * simple logger and version
 */
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include <maya/MString.h>
#include <maya/MGlobal.h>

#include "utils.h"

#ifdef WIN32
#define snprintf _snprintf
#endif
using namespace std;

#define MaxStringLength 8192

namespace
{
static int s_LogLine;
static const char * s_LogFile;
static const char * s_LogFunc;
static int s_LogLevel = DEFAULT_LOG_LEVEL;

void __Log(const char * a_Str)
{
   MGlobal::displayWarning(MString(a_Str));
   cout << a_Str << endl;
#ifdef WIN32
   OutputDebugStringA( a_Str );
   OutputDebugStringA( "\n" );
#endif
}

void _LogInt(const char * a_Format, va_list a_VA)
{
   char buf[ MaxStringLength];
   vsnprintf(buf, MaxStringLength, a_Format, a_VA);
   __Log(buf);
}

void _LogInt(const char * a_Format, ...)
{
   va_list va;
   va_start(va, a_Format);
   _LogInt(a_Format, va);
   va_end(va);
}

void _Log(const char * a_Category, const char * a_Format, va_list a_VA)
{
   char buf0[ MaxStringLength];
   char buf1[ MaxStringLength];
   vsnprintf(buf0, MaxStringLength, a_Format, a_VA);
   snprintf(buf1, MaxStringLength, "%s: [%20s:%-4i:%-12s()] %s", a_Category,
         s_LogFile, s_LogLine, s_LogFunc, buf0);
   __Log(buf1);
}
} // end of anon namespace

int LOG_LEVEL()
{
   return s_LogLevel;
}

void LOG_LEVEL(int a_LogLevel)
{
   s_LogLevel = a_LogLevel;
}

bool AssertFailure(const char * a_Txt, const char * a_Msg, const char * a_File,
      unsigned int a_Line)
{
   char buf[ MaxStringLength];
   snprintf(buf, MaxStringLength, "%s\n%s\nFAILED IN FILE [%s:%d]\n", a_Msg,
         a_Txt, a_File, a_Line);
   __Log(buf);
   return true;
}

const char *
AssertFailureFormat(const char * a_Format, ...)
{
   static char str[ MaxStringLength];
   va_list va;
   va_start(va, a_Format);
   vsnprintf(str, MaxStringLength, a_Format, va);
   va_end(va);
   return str;
}

void _SetLogData(unsigned int a_Line, const char * a_File,
      const char * a_Function)
{
   s_LogLine = a_Line;
   s_LogFile = a_File;
   s_LogFunc = a_Function;
}

#define IMPLEMENT_CATEGORY(CAT)                                               \
    void CAT( const char * a_Format, ... )                                    \
    {                                                                         \
        va_list va;                                                           \
        va_start( va, a_Format );                                             \
        _Log( #CAT, a_Format, va );                                           \
        va_end( va );                                                         \
    }

IMPLEMENT_CATEGORY(DEBG);
IMPLEMENT_CATEGORY(INFO);
IMPLEMENT_CATEGORY(WARN);
IMPLEMENT_CATEGORY(ERRR);
IMPLEMENT_CATEGORY(PRNT);

string Format(const char * a_Fmt, ...)
{
   char str[ MaxStringLength];
   va_list va;
   va_start(va, a_Fmt);
   vsnprintf(str, MaxStringLength, a_Fmt, va);
   va_end(va);
   return string(str);
}

