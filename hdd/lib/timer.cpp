#include "timer.h"

bool Timer::enabled = true;
int Timer::data[TimeNone] = {0};

long unsigned int Timer::microtime()
{
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return (long unsigned int) (1e6*tv.tv_sec + tv.tv_usec);
}
void Timer::start(TimeIds id)
{
	if (!enabled) {
		return;
	}
	data[id] -= microtime();
}
void Timer::stop(TimeIds id)
{
	if (!enabled) {
		return;
	}
	data[id] += microtime();
}
float Timer::secs(TimeIds id)
{
	return data[id] * 1e-6;
}
