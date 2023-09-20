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
		++m_currentParticleCount;
		if (m_currentParticleCount >= m_particleMaxCount)
		{
			m_currentParticleCount = 0;
		}

		if (m_isActive[m_currentParticleCount])
		{
			//full
			m_currentParticleCount = -1;
		}
		else
		{
			m_particleIndex = m_currentParticleCount;
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
	m_velocity[index] = cd::Vec3f(1.0f, 1.0f, 1.0f);
	m_acceleration[index] = cd::Vec3f(1.0f, 0.0f, 0.0f);

	m_color[index] = cd::Vec3f{0.0f, 0.0f,1.0f};

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

	if (index % 4 == 0)
	{
		m_pos[index].x() = m_pos[index].x() + m_velocity[index].x() * deltaTime + 0.5f * m_acceleration[index].x() * deltaTime * deltaTime;
		m_pos[index].y() = m_pos[index].y() + m_velocity[index].y() * deltaTime + 0.5f * m_acceleration[index].y() * deltaTime * deltaTime;
		m_texture_u[index] = 0.0f;
		m_texture_v[index] = 0.0f;
	}
	else if (index % 4 == 1)
	{
		m_pos[index].x() = m_pos[index-1].x()+ 1 + m_velocity[index].x() * deltaTime + 0.5f * m_acceleration[index].x() * deltaTime * deltaTime;
		m_pos[index].y() = m_pos[index-1].y() + m_velocity[index].y() * deltaTime + 0.5f * m_acceleration[index].y() * deltaTime * deltaTime;
		m_texture_u[index] = 1.0f;
		m_texture_v[index] = 0.0f;
	}
	else if (index % 4 == 2)
	{
		m_pos[index].x() = m_pos[index-2].x() + 1+ m_velocity[index].x() * deltaTime + 0.5f * m_acceleration[index].x() * deltaTime * deltaTime;
		m_pos[index].y() = m_pos[index-2].y() + 1 + m_velocity[index].y() * deltaTime + 0.5f * m_acceleration[index].y() * deltaTime * deltaTime;
		m_texture_u[index] = 1.0f;
		m_texture_v[index] = 1.0f;
	}
	else if (index % 4 == 3)
	{
		m_pos[index].x() = m_pos[index-3].x() + m_velocity[index].x() * deltaTime + 0.5f * m_acceleration[index].x() * deltaTime * deltaTime;
		m_pos[index].y() = m_pos[index-3].y() + 1 +  m_velocity[index].y() * deltaTime + 0.5f * m_acceleration[index].y() * deltaTime * deltaTime;
		m_texture_u[index] = 0.0f;
		m_texture_v[index] = 1.0f;
	}

	for (int i = index; i < m_particleMaxCount; ++i)
	{
		m_currentTime[i] += deltaTime;
	}
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
	 m_pos.resize(m_particleMaxCount);
	 m_velocity.resize(m_particleMaxCount);
	 m_acceleration.resize(m_particleMaxCount);
	 m_color.resize(m_particleMaxCount);
	 m_texture_u.resize(m_particleMaxCount);
	 m_texture_v.resize(m_particleMaxCount);

	m_isActive.resize(m_particleMaxCount);
	m_currentTime.resize(m_particleMaxCount);
	m_lifeTime.resize(m_particleMaxCount);
}
