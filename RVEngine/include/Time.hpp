#pragma once

#include <chrono>

class TimeUntil
{
public:
	TimeUntil();
	TimeUntil(float time);
	void operator = (float time);
	explicit operator bool() const;

	float GetPassed();
	float GetFraction();
	float GetRemaining();
	float GetFractionRemaining();
private:
	std::chrono::system_clock::time_point startTime;
	std::chrono::system_clock::time_point targetTime;
	float m_Time = 0;
};