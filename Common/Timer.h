#pragma once
#include <ctime>
#include <chrono>
struct Timer {
	Timer();
	double elapsed();
	void restart();

private:
	std::chrono::high_resolution_clock::time_point m_start;
};