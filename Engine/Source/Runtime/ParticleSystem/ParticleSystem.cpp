#include "ParticleSystem.h"

void engine::ParticleSystem::AllocateParticleIndex()
{
	if (!m_FreeParticleIndex.empty())
	{
		int index = m_FreeParticleIndex.back();
		m_FreeParticleIndex.pop_back();
		m_particleIndex = index;
	}
	else
	{
		++m_particleIndex;
		if (m_particleIndex >= m_particleMaxCount)
		{
			m_particleIndex = 0;
		}

		if (m_isActive[m_particleIndex])
		{
			//full
			m_particleIndex = -1;
		}
	}

	if (m_particleIndex != -1)
	{
		Reset(m_particleIndex);
		Active(m_particleIndex);
	}
}

void engine::ParticleSystem::Reset(int index)
{
	m_pos[index] = cd::Vec3f(0.0f, 0.0f, 0.0f);
	m_velocity[index] = cd::Vec3f(rand() % 41, 0.0f, 0.0f);
	m_acceleration[index] = cd::Vec3f(0.0f, 0.0f, 0.0f);

	m_color[index] = cd::Vec3f{rand() %30, rand() %45,0.0f};

	m_isActive[index] = false;
	m_currentTime[index] = 0.0f;
	m_lifeTime[index] = 6.0f;

}

void engine::ParticleSystem::Update(float deltaTime, int index)
{
	if (m_currentTime[index] >= m_lifeTime[index])
	{
		m_isActive[index] = false;
		return;
	}

	//x = v0t +1/2 at*2
	m_pos[index].x() = m_pos[index].x() + m_velocity[index].x() * deltaTime + 0.5f * m_acceleration[index].x() * deltaTime * deltaTime;
	m_pos[index].y() = m_pos[index].y() + m_velocity[index].y() * deltaTime + 0.5f * m_acceleration[index].y() * deltaTime * deltaTime;
	m_pos[index].z() = m_pos[index].z() + m_velocity[index].z() * deltaTime + 0.5f * m_acceleration[index].z() * deltaTime * deltaTime;

	m_velocity[index].x() += m_acceleration[index].x() * deltaTime;
	m_velocity[index].y() += m_acceleration[index].y() * deltaTime;
	m_velocity[index].z() += m_acceleration[index].z() * deltaTime;

	m_currentTime[index] += deltaTime;
}

void engine::ParticleSystem::UpdateActive(float deltaTime)
{
	for (int i = 0; i < m_particleMaxCount; ++i)
	{
		if (!m_isActive[i])
		{
			continue;
		}

		Update(deltaTime, i);
		if (!m_isActive[i])
		{
			m_FreeParticleIndex.push_back(i);
		}
		else
		{
			++m_currentActiveCount;
		}
	}
}

void engine::ParticleSystem::Init()
{
	 m_FreeParticleIndex.resize(m_particleMaxCount);
	 m_pos.resize(m_particleMaxCount);
	 m_velocity.resize(m_particleMaxCount);
	 m_acceleration.resize(m_particleMaxCount);
	 m_color.resize(m_particleMaxCount);

	m_isActive.resize(m_particleMaxCount);
	m_currentTime.resize(m_particleMaxCount);
	m_lifeTime.resize(m_particleMaxCount);
}
