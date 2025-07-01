#include "Time.hpp"

TimeUntil::TimeUntil()
{

}

TimeUntil::TimeUntil(float time)
{
	startTime = std::chrono::system_clock::now();
	targetTime = std::chrono::system_clock::now() + std::chrono::milliseconds(static_cast<int>(time * 1000));
	m_Time = time;
}

void TimeUntil::operator=(float time)
{
	startTime = std::chrono::system_clock::now();
	targetTime = std::chrono::system_clock::now() + std::chrono::milliseconds(static_cast<int>(time * 1000));
	m_Time = time;
}

TimeUntil::operator bool()
{
	return GetPassed() >= m_Time;
}

// Returns elapsed time in seconds
float TimeUntil::GetPassed()
{
	 return std::chrono::duration<float>(std::chrono::system_clock::now() - startTime).count();
}

// Returns fraction of time passed
float TimeUntil::GetFraction()
{
	return GetPassed() / m_Time;
}

// Returns remaining time in seconds
float TimeUntil::GetRemaining()
{
	return m_Time - GetPassed();
}

// Returns fraction of time remaining
float TimeUntil::GetFractionRemaining()
{
	return GetRemaining() / m_Time;
}


