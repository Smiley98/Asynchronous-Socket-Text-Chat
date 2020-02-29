#pragma once
#include <ctime>
struct Timer {
	Timer();
	double elapsed();
	void restart();

private:
	clock_t m_start;
	clock_t m_end;
};