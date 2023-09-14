#pragma once

#include "Base/Template.h"
#include "Core/StringCrc.h"
#include "Math/Vector.hpp"
#include <vector>

namespace engine
{

class ParticleSystem final
{
public:
	ParticleSystem() = default;
	ParticleSystem(const ParticleSystem&) = default;
	ParticleSystem& operator=(const ParticleSystem&) = default;
	ParticleSystem(ParticleSystem&&) = default;
	ParticleSystem& operator=(ParticleSystem&&) = default;
	~ParticleSystem() = default;

	int& GetIndex() { return m_particleIndex; }

	int& GetMaxCount() { return m_particleMaxCount; }

	int& GetParticleActiveCount() { return m_currentActiveCount; }

	cd::Vec3f& GetPos(int index) { return m_pos[index]; }
	void SetPos(cd::Vec3f pos) { m_pos[m_particleIndex] = pos; }

	cd::Vec3f& GetVelocity(int index) { return m_velocity[index]; }
	void SetVelocity(cd::Vec3f velocity) { m_velocity[m_particleIndex] = velocity; }

	cd::Vec3f& GetAcceleration(int index) { return m_acceleration[index]; }
	void SetAcceleration(cd::Vec3f acceleration) { m_acceleration[m_particleIndex] = acceleration; }

	cd::Vec3f& GetColor(int index) { return m_color[index]; }
	void SetColor(cd::Vec3f color) { m_color[m_particleIndex] = color; }

	void  Active(int index) { m_isActive[index] = true; }
	bool IsActive(int index) { return m_isActive[index]; }

	float& GetLifeTime(int index) { return m_lifeTime[index]; }
	void SetLifeTime(float lifeTime) { m_lifeTime[m_particleIndex] = lifeTime; }

	void AllocateParticleIndex();

	void Reset( int index);

	void Update(float deltaTime, int index);

	void UpdateActive(float deltaTime);

	void Init();

private:
	int m_particleIndex = -1;
	int m_particleMaxCount = 300;
	int m_currentActiveCount = 0;
	
	std::vector<int> m_FreeParticleIndex;
	std::vector<cd::Vec3f> m_pos;
	std::vector<cd::Vec3f> m_velocity;
	std::vector<cd::Vec3f> m_acceleration;
	std::vector<cd::Vec3f> m_color;

	std::vector<bool>	m_isActive;
	std::vector<float> m_currentTime;
	std::vector<float> m_lifeTime;
};

}