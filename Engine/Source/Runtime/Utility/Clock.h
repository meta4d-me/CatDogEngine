#pragma once

#include <string>
#include <chrono>

namespace engine::Tools
{
	class Clock
	{
	private:
		bool	m_IsInit = false;
		float	m_TimeScale = 1.0f;
		float	m_DeltaTime = 0.0f;
		float	m_TimeSinceStart = 0.0f;

		std::chrono::duration<double>			m_Elapsed;
		std::chrono::steady_clock::time_point	m_LastTime;
		std::chrono::steady_clock::time_point	m_StartTime;
		std::chrono::steady_clock::time_point	m_CurrentTime;

	public:
		void Update();
		float GetFramerate();
		float GetDeltaTime();
		float GetDeltaTimeUnscaled();
		float GetTimeSinceStart();
		float GetTimeScale();
		void Scale(float p_coeff);
		void SetTimeScale(float p_timeScale);

	private:
		void Initialize();
	};
}