#pragma once

#include <chrono>
#include <filesystem>

namespace engine
{

class Clock
{
public:
	// C++20 only
	static long long FileTimePointToTimeStamp(std::filesystem::file_time_type fileTime);
	// C++20 only
	static std::filesystem::file_time_type TimeStampToFileTimePoint(long long timeStamp);

public:
	Clock();
	Clock(const Clock&) = default;
	Clock& operator=(const Clock&) = default;
	Clock(Clock&&) = default;
	Clock& operator=(Clock&&) = default;

	void Update();
	float GetFramerate() const { return 1.0f / m_deltaTime; }
	float GetDeltaTime() const { return m_deltaTime; }
	float GetTimeSinceStart() const { m_timeSinceStart; }

private:
	float m_deltaTime = 0.0f;
	float m_timeSinceStart = 0.0f;

	std::chrono::duration<float> m_elapsed;
	std::chrono::steady_clock::time_point m_lastTime;
	std::chrono::steady_clock::time_point m_startTime;
	std::chrono::steady_clock::time_point m_currentTime;
};

}