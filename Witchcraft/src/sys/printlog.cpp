#include "printlog.h"

#include "paths.h"

#include <iostream>
#include <fstream>
#include <queue>
#include <cstdio>
#include <cstdarg>
using namespace std;

#include "tools/fixedstring.h"

namespace {

//ofstream logfile;
FILE* logfile = nullptr;
vector<FixedString<64>> crash_reports;
queue<FixedString<64>> console_queue;

FixedString<64> last_message;

struct
{
	int stdout_sensitivity = 0;
	int logfile_sensitivity = 0;
	int console_sensitivity = 0;
} config;

} // namespace <anon>


namespace plog {

void Print(int severity, const char* prefix, const char* fmt, va_list args)
{
	FixedString<64> instr;
	vsnprintf(instr.c_str, 64, fmt, args);

	if (last_message == instr)
		return;
	else
		last_message = instr;

	if (logfile == nullptr)
	{
		char path[270];
		snprintf(path, 270, "%s%s", sys::getUserPath().c_str(), LOG_FILENAME);
		logfile = fopen(path, "w");
		// TODO: Load config
	}

	if (severity > config.console_sensitivity)
		{ console_queue.push(instr); }

	// TODO: Strip tags from the message being printed.

	if (severity > config.stdout_sensitivity)
	{ 
		if (prefix) printf(prefix);
		vprintf(fmt, args);
	}

	if (severity > config.logfile_sensitivity)
	{
		if (prefix) fprintf(logfile, prefix);
		vfprintf(logfile, fmt, args);
	}

	if (severity == LOG_SEVERITY_FATAL)
	{
		crash_reports.push_back(instr);
	}
}

bool PopConsoleQueue(string& out)
{
	if (console_queue.empty())
		return false;

	out = console_queue.front().c_str;
	console_queue.pop();
	return true;
}

#ifdef PLATFORM_WIN32
#include <Windows.h>
void ShowCrashReports()
{
	for (auto& str : crash_reports)
	{
		MessageBox(nullptr, utf8_to_utf16(str.c_str).c_str(), L"Fatal Error", MB_OK | MB_ICONERROR);
	}
}
#endif // PLATFORM_WIN32

} // namespace plog