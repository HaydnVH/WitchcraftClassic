#ifdef PLATFORM_WIN32
#include "timer.h"

#include <Windows.h>

namespace sys {

uint64_t Timer::ticks_per_second = 0;

Timer::Timer()
{
	if (ticks_per_second == 0)
	{
		LARGE_INTEGER freq;
		QueryPerformanceFrequency(&freq);
		ticks_per_second = freq.QuadPart;
	}

	LARGE_INTEGER ctr;
	QueryPerformanceCounter(&ctr);
	current_ticks = ctr.QuadPart;
	previous_ticks = current_ticks;
}

void Timer::update()
{
	LARGE_INTEGER ctr;
	QueryPerformanceCounter(&ctr);
	previous_ticks = current_ticks;
	current_ticks = ctr.QuadPart;
}

} // namespace sys

#endif // PLATFORM_WIN32