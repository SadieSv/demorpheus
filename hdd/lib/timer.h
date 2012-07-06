#ifndef TIMER_H
#define TIMER_H

#include <cstdlib>
#include <sys/time.h>

enum TimeIds {
	TimeTotal,
	TimeLoad,
	TimeFind,
	TimeFindMemoryAndJump,
	TimeLaunches,
	TimeBackwardsTraversal,
	TimeEmulatorStart,
	TimeNone
};

/**
@brief
Calculate time
*/

class Timer {
public:
	static bool enabled;
	static void start(TimeIds id = TimeTotal);
	static void stop(TimeIds id = TimeTotal);
	static float secs(TimeIds id = TimeTotal);
	static int data[TimeNone];
	static long unsigned int microtime();
};

#endif 
