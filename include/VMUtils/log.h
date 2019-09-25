#ifndef _ERROR_H_
#define _ERROR_H_

#include <string>
#include <cstdarg>
#include <cassert>
#include "utils_config.h"

#ifdef NDEBUG
#define Assert(expr) ((void)(0)) 
#else
#define Assert(expr) (expr == true?((void)(0))(assert(expr)))
#endif

namespace ysl
{
	VMUTILS_EXPORTS std::string formatToString(const std::string& fmt, va_list args);

	VMUTILS_EXPORTS void _internal_msg_process_(const char* format, va_list args, const char* type,bool);

	VMUTILS_EXPORTS void Error(const char* fmt, ...);

	VMUTILS_EXPORTS void Warning(const char* fmt, ...);

	VMUTILS_EXPORTS void Log(const char * fmt, ...);

	VMUTILS_EXPORTS void Display(const char * fmt, ...);

	inline
	void Debug(const char * fmt, ...)
	{
#ifndef NDEBUG
		va_list args;
		va_start(args, fmt);
		_internal_msg_process_(fmt, args, "Debug",false);
		va_end(args);
#else
		void(0);
#endif	
	}
}

#endif