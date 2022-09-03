#include "Clock.h"

namespace engine::Tools
{
	float Clock::GetFramerate()
	{
		return 1.0f / (m_DeltaTime);
	}

	float Clock::GetDeltaTime()
	{
		return m_DeltaTime * m_TimeScale;
	}

	float Clock::GetDeltaTimeUnscaled()
	{
		return m_DeltaTime;
	}

	float Clock::GetTimeSinceStart()
	{
		return m_TimeSinceStart;
	}

	float Clock::GetTimeScale()
	{
		return m_TimeScale;
	}

	void Clock::Scale(float p_coeff)
	{
		m_TimeScale *= p_coeff;
	}

	void Clock::SetTimeScale(float p_timeScale)
	{
		m_TimeScale = p_timeScale;
	}

	void Clock::Initialize()
	{
		m_DeltaTime = 0.0f;

		m_StartTime = std::chrono::steady_clock::now();
		m_CurrentTime = m_StartTime;
		m_LastTime = m_StartTime;

		m_IsInit = true;
	}

	void Clock::Update()
	{
		m_LastTime = m_CurrentTime;
		m_CurrentTime = std::chrono::steady_clock::now();
		m_Elapsed = m_CurrentTime - m_LastTime;

		if (m_IsInit)
		{
			m_DeltaTime = m_Elapsed.count() > 0.1 ? 0.1f : static_cast<float>(m_Elapsed.count());
			m_TimeSinceStart += m_DeltaTime * m_TimeScale;
		} else {
			Initialize();
		}
	}
}