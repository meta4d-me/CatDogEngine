#include "Clock.h"

namespace engine
{

long long Clock::FileTimePointToTimeStamp(std::filesystem::file_time_type fileTime)
{
	auto systemClock = std::chrono::clock_cast<std::chrono::system_clock>(fileTime);
	return static_cast<long long>(std::chrono::system_clock::to_time_t(systemClock));
}

std::filesystem::file_time_type Clock::TimeStampToFileTimePoint(long long timeStamp)
{
	auto systemClock = std::chrono::system_clock::from_time_t(static_cast<time_t>(timeStamp));
	return std::chrono::clock_cast<std::chrono::file_clock>(systemClock);
}

Clock::Clock() :
	m_elapsed{}
{
	m_startTime = std::chrono::steady_clock::now();
	m_currentTime = m_startTime;
	m_lastTime = m_startTime;
}

void Clock::Update()
{
	m_lastTime = m_currentTime;
	m_currentTime = std::chrono::steady_clock::now();
	m_elapsed = m_currentTime - m_lastTime;

	m_deltaTime = m_elapsed.count();
	m_timeSinceStart += m_deltaTime;
}

}