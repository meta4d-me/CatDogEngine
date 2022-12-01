#include "Clock.h"

namespace engine
{

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