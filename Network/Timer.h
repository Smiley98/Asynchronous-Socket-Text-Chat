#pragma once
#include <ctime>
struct Timer {
	Timer() {
		restart();
	}

	double elapsed() {
		return (clock() - m_start) / (CLOCKS_PER_SEC / 1000);
	}

	void restart() {
		m_start = clock();
	}

private:
	clock_t m_start;
	clock_t m_end;
};