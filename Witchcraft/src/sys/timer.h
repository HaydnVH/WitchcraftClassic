#ifndef HVH_WC_SYS_TIMER_H
#define HVH_WC_SYS_TIMER_H

#include <cstdint>

namespace sys {

class Timer
{
public:
	Timer();

	void update();

	// Returns the amount of time (in seconds) that has passed between the start of the program and the most recent call to update().
	double getTime()
		{ return (double)current_ticks / (double)ticks_per_second; }

	// Returns the amount of time (in seconds) that has passed between the most recent call to update() and the previous call to update().
	double getDeltaTime()
		{ return (double)(current_ticks - previous_ticks) / (double)ticks_per_second; }

private:
	uint64_t current_ticks;
	uint64_t previous_ticks;
	static uint64_t ticks_per_second;
};

} // namespace sys

#endif // HVH_WC_SYS_TIMER_H