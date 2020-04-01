#include "Timer.h"

Timer::Timer()
{
	restart();
}

double Timer::elapsed()
{
	using sec = std::chrono::duration<double>;
	return std::chrono::duration_cast<sec>(std::chrono::high_resolution_clock::now() - m_start).count();
}

void Timer::restart()
{
	m_start = std::chrono::high_resolution_clock::now();
}
