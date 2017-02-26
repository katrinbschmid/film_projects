#ifndef UTILS_H_
#define UTILS_H_

#include <stdio.h>
#include <string>

// change this to reflect what logging level you want
#define DEFAULT_LOG_LEVEL 3

int LOG_LEVEL();
void LOG_LEVEL(int l);

bool AssertFailure(const char * a_Txt, const char * a_Msg, const char * a_File,
      unsigned int a_Line);
const char * AssertFailureFormat(const char * a_Format, ...);

#define ASSERTM(exp,msg) if (!(exp)) { if ( AssertFailure( #exp, AssertFailureFormat msg, __FILE__, __LINE__ ) ) {} } else {}

#define DEBG_LEVEL 1
#define INFO_LEVEL 2
#define WARN_LEVEL 3
#define ERRR_LEVEL 4
// Always print
#define PRNT_LEVEL 100
#define T_EPSILON 0.0001f

inline void HandleGlError(int a_Error)
{
   a_Error;
#ifdef WIN32
//	__asm { int 3 }
#endif
}

void DEBG(const char * a_Format, ...);
void INFO(const char * a_Format, ...);
void WARN(const char * a_Format, ...);
void ERRR(const char * a_Format, ...);
void PRNT(const char * a_Format, ...);

void _SetLogData(unsigned int a_Line, const char * a_File,
      const char * a_Function);
#define LOG(CAT,X) if ( CAT##_LEVEL >= LOG_LEVEL() ) { _SetLogData( __LINE__, __FILE__, __FUNCTION__ ); CAT X ; } else { /*fprintf( stderr, "Wrong log level: %d>=%d\n", CAT##_LEVEL, LOG_LEVEL() );*/ }
#define LOG_IF_GL_ERROR(CAT,X)                                                              \
if ( CAT##_LEVEL >= LOG_LEVEL() ) {                                                         \
		int error = glGetError();                                                           \
		if ( error != GL_NO_ERROR ) {                                                       \
			HandleGlError( error );															\
			LOG( CAT, X );                                                                  \
			LOG( CAT, ( "openGL ERROR, possible cause: [%i] [%s]",                          \
				error, gluErrorString( error ) ) );                                         \
		}                                                                                   \
} else {}

template<typename T>
T MapToRange(T a_Min, T a_Range, T a_V)
{
   return (a_V - a_Min) / a_Range;
}

template<typename T>
bool NearZero(T a_V)
{
   return -T_EPSILON < a_V && a_V < T_EPSILON;
}

template<typename T>
bool IsInBetween(T a_Min, T a_Max, T a_V)
{
   return a_Min < a_V && a_V < a_Max;
}

std::string Format(const char * a_Fmt, ...);

#endif /* UTILS_H_ */
