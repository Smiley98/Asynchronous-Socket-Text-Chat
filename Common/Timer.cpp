#include "Timer.h"

Timer::Timer()
{
	restart();
}

double Timer::elapsed()
{
	return (clock() - m_start) / (CLOCKS_PER_SEC / 1000);
}

void Timer::restart()
{
	m_start = clock();
}
