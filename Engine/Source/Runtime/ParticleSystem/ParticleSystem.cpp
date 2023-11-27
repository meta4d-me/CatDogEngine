#include "ParticleSystem.h"
#include <fstream>
#include <random>

void engine::ParticleSystem::AllocateParticleIndex()
{
	if (!m_freeParticleIndex.empty())
	{
		int index = m_freeParticleIndex.back();
		m_freeParticleIndex.pop_back();
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
	m_rotation[index] = cd::Vec3f(0.0f, 0.0f, 0.0f);
	m_scale[index] = cd::Vec3f(0.0f, 0.0f, 0.0f);
	m_velocity[index] = cd::Vec3f(0.0f, 0.0f, 0.0f);
	m_velocityXYZ[index] = cd::Vec3f(0.0f, 0.0f, 0.0f);
	m_acceleration[index] = cd::Vec3f(0.0f, 0.0f, 0.0f);
	
	m_color[index] = cd::Vec4f{1.0f, 1.0f,1.0f,1.0f};

	m_isActive[index] = false;
	m_currentTime[index] = 0.0f;
	m_lifeTime[index] = 6.0f;

}

void engine::ParticleSystem::Update(float deltaTime, int index)
{
	if (m_currentTime[index] >= m_lifeTime[index])
	{
		m_isActive[index] = false;
		m_currentTime[index] = 0.0f;
		return;
	}

	if (GetType() == engine::ParticleType::Sprite)
	{
		if (GetRandomState()&& index%4 ==0)
		{
			std::random_device rd; 
			std::default_random_engine generator(rd()); 
			std::uniform_real_distribution<float> distributionX(std::min(-m_twoSideVelocity.x(), m_twoSideVelocity.x()), std::max(-m_twoSideVelocity.x(), m_twoSideVelocity.x()));
			std::uniform_real_distribution<float> distributionY(std::min(-m_twoSideVelocity.y(), m_twoSideVelocity.y()), std::max(-m_twoSideVelocity.y(), m_twoSideVelocity.y()));
			//std::uniform_real_distribution<float> distributionZ(std::min(-m_twoSideVelocity.x(), m_twoSideVelocity.x()), std::max(-m_twoSideVelocity.x(), m_twoSideVelocity.x()));
			float randomX = distributionX(generator);
			float randomY = distributionY(generator);
			m_velocityXYZ[index].x() += randomX;
			m_velocityXYZ[index].y() += randomY;
		}

		if (index % 4 == 0)
		{
			m_pos[index].x() = m_pos[index].x() + m_velocity[index].x() + m_velocityXYZ[index].x();
			m_pos[index].y() = m_pos[index].y() + m_velocity[index].y() + m_velocityXYZ[index].y();
			m_texture_uv[index].x() = 1.0f;
			m_texture_uv[index].y() = 1.0f;	
		}
		else if (index % 4 == 1)
		{
			m_pos[index].x() = m_pos[index-1].x()+ m_scale[index].x() ;
			m_pos[index].y() = m_pos[index-1].y();
			m_texture_uv[index].x() = 0.0f;
			m_texture_uv[index].y() = 1.0f;	
		}
		else if (index % 4 == 2)
		{
			m_pos[index].x() = m_pos[index-2].x() + m_scale[index].x();
			m_pos[index].y() = m_pos[index-2].y() + m_scale[index].y();
			m_texture_uv[index].x() = 0.0f;
			m_texture_uv[index].y() = 0.0f;
		}
		else if (index % 4 == 3)
		{
			m_pos[index].x() = m_pos[index-3].x();
			m_pos[index].y() = m_pos[index-3].y() + m_scale[index].y() ;
			m_texture_uv[index].x() = 1.0f;
			m_texture_uv[index].y() = 0.0f;
		}
	}
	else if (GetType() == engine::ParticleType::Ribbon)
	{

	}
	else if (GetType() == engine::ParticleType::Track)
	{

	}
	else if (GetType() == engine::ParticleType::Ring)
	{

	}
	else if (GetType() == engine::ParticleType::Model)
	{

	}

	for (int i = index; i < m_particleMaxCount; ++i)
	{
		m_currentTime[i] += deltaTime;
	}
}

bool engine::ParticleSystem::UpdateActive(float deltaTime,int i)
{
	if (!m_isActive[i])
	{
		return false;
	}

	Update(deltaTime, i);
	if (!m_isActive[i])
	{
		m_freeParticleIndex.push_back(i);
	}
	else
	{
		++m_currentActiveCount;
	}
	return true;
}

void engine::ParticleSystem::Init()
{
	m_pos.resize(m_particleMaxCount);
	m_rotation.resize(m_particleMaxCount);
	m_scale.resize(m_particleMaxCount);
	m_velocity.resize(m_particleMaxCount);
	m_velocityXYZ.resize(m_particleMaxCount);
	m_acceleration.resize(m_particleMaxCount);
	m_color.resize(m_particleMaxCount);
	m_texture_uv.resize(m_particleMaxCount);

	m_isActive.resize(m_particleMaxCount);
	m_currentTime.resize(m_particleMaxCount);
	m_lifeTime.resize(m_particleMaxCount);
}

void engine::ParticleSystem::SetMaxCount(int num)
{
	m_particleMaxCount = num;
	//m_currentParticleCount = 0;
	//m_particleIndex = -1;
	//m_currentActiveCount = 0;
	//Init();
	//for (int i = 0; i < m_particleMaxCount; ++i)
	//{
	//	Reset(i);
	//}
}
