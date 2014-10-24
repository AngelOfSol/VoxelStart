#pragma once
#include <chrono>
class timer
{
private:
	std::chrono::steady_clock m_clock;

	std::chrono::system_clock::time_point m_start;
	std::chrono::system_clock::time_point m_end;
public:
	timer();
	~timer();
	void start()
	{
		this->m_start = this->m_clock.now();
	}
	void stop()
	{
		this->m_end = this->m_clock.now();
	}

	auto duration() const -> std::chrono::system_clock::duration
	{
		auto diff = this->m_end - this->m_start;
		return diff;
	}

	auto lap() const -> std::chrono::system_clock::duration
	{
		return this->m_clock.now() - this->m_start;
	}


};

