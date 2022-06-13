#ifndef HVH_WC_SYS_PRINTLOG_H
#define HVH_WC_SYS_PRINTLOG_H

#include "tools/stringhelper.h"
#include <cstdarg>

namespace plog {

constexpr const char* LOG_FILENAME = "log.txt";

enum PrintlogSeverity
{
	LOG_SEVERITY_INFO = 1,
	LOG_SEVERITY_WARNING,
	LOG_SEVERITY_ERROR,
	LOG_SEVERITY_FATAL,
	LOG_SEVERITY_USER,
	LOG_SEVERITY_NEVER
};

constexpr const char* INFOMARK = "INFO: ";
constexpr const char* INFOMORE = "     - ";
constexpr const char* WARNMARK = "WARNING: ";
constexpr const char* WARNMORE = "        ~ ";
constexpr const char* ERRMARK = "ERROR!: ";
constexpr const char* ERRMORE = "       ! ";
constexpr const char* FATALMARK = "FATAL ERROR!: ";

void Print(int severity, const char* prefix, const char* fmt, va_list args);
bool PopConsoleQueue(std::string& out);
void ShowCrashReports();

inline void info(const char* fmt, ...)
	{ va_list args; va_start(args, fmt); Print(LOG_SEVERITY_INFO, INFOMARK, fmt, args); va_end(args); }

inline void infomore(const char* fmt, ...)
	{ va_list args; va_start(args, fmt); Print(LOG_SEVERITY_INFO, INFOMORE, fmt, args); va_end(args); }

inline void warning(const char* fmt, ...)
	{ va_list args; va_start(args, fmt); Print(LOG_SEVERITY_WARNING, WARNMARK, fmt, args); va_end(args); }

inline void warnmore(const char* fmt, ...)
	{ va_list args; va_start(args, fmt); Print(LOG_SEVERITY_WARNING, WARNMORE, fmt, args); va_end(args); }

inline void error(const char* fmt, ...)
	{ va_list args; va_start(args, fmt); Print(LOG_SEVERITY_ERROR, ERRMARK, fmt, args); va_end(args); }

inline void errmore(const char* fmt, ...)
	{ va_list args; va_start(args, fmt); Print(LOG_SEVERITY_ERROR, ERRMORE, fmt, args); va_end(args); }

inline void fatal(const char* fmt, ...)
	{ va_list args; va_start(args, fmt); Print(LOG_SEVERITY_FATAL, FATALMARK, fmt, args); va_end(args); }

inline void print(const char* fmt, ...)
	{ va_list args; va_start(args, fmt); Print(LOG_SEVERITY_USER, nullptr, fmt, args); va_end(args); }

} // namespace plog

#endif // HVH_WC_SYS_ERRORLOG_H