/*#pragma once
#include <array>
#include <memory>

template<typename T, size_t count>
class StaticStack :
	public std::array<T, count>
{
public:
	void push_back(const int& element) {

	}
	void pop_back() {

	}
private:
	//I would have to overload nearly every method of array to make this work otherwise m_count wouldn't accurately reflect the contents of the array.
	size_t m_count;
};

#include <mutex>
template<typename T, size_t count>
class ConcurrentStaticStack :
	public StaticStack<T, count>
{
public:

private:
	std::mutex m_mutex;
};*/