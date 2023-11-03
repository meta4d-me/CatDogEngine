#pragma once

#include "Base/Template.h"
#include "Core/StringCrc.h"
#include "Math/Vector.hpp"
#include <vector>
#include "ECWorld/TransformComponent.h"

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

	cd::Vec3f& GetRotation(int index) { return m_rotation[index]; }
	void SetRotation(cd::Vec3f rotate) { m_rotation[m_particleIndex] = rotate; }

	cd::Vec3f& GetScale(int index) { return m_scale[index]; }
	void SetScale(cd::Vec3f scale) { m_scale[m_particleIndex] = scale; }

	cd::Vec3f& GetVelocity() { return m_velocity[m_particleIndex]; }
	void SetVelocity(cd::Vec3f velocity) { m_velocity[m_particleIndex] = velocity; }

	//cd::Quaternion& GetEmiterDirection( ) { return m_emiterDirection[m_particleIndex]; }
	//void SetEmiterDirection(cd::Quaternion direction) { m_emiterDirection[m_particleIndex] = direction; }

	cd::Vec3f& GetAcceleration(int index) { return m_acceleration[index]; }
	void SetAcceleration(cd::Vec3f acceleration) { m_acceleration[m_particleIndex] = acceleration; }

	cd::Vec4f& GetColor(int index) { return m_color[index]; }
	void SetColor(cd::Vec4f color) { m_color[m_particleIndex] = color; }

	float& GetTexture_u(int index) { return m_texture_u[index]; }
	float& GetTexture_v(int index) { return m_texture_v[index]; }

	void  Active(int index) { m_isActive[index] = true; }
	bool IsActive(int index) { return m_isActive[index]; }

	float& GetLifeTime(int index) { return m_lifeTime[index]; }
	void SetLifeTime(float lifeTime) { m_lifeTime[m_particleIndex] = lifeTime; }

	void AllocateParticleIndex();

	void Reset( int index);

	void Update(float deltaTime, int index);

	bool UpdateActive(float deltaTime, int i);

	void Init();

private:
	int m_currentParticleCount = 0;
	int m_particleIndex = -1;
	int m_particleMaxCount = 300;
	int m_currentActiveCount = 0;

	std::vector<int> m_FreeParticleIndex;
	std::vector<cd::Vec3f> m_pos;
	std::vector<cd::Vec3f>m_rotation;
	std::vector<cd::Vec3f>m_scale;

	std::vector<cd::Vec3f> m_velocity;
	//std::vector<cd::Quaternion> m_emiterDirection;
	std::vector<cd::Vec3f> m_acceleration;
	std::vector<cd::Vec4f> m_color;
	std::vector<float> m_texture_u;
	std::vector<float> m_texture_v;

	std::vector<bool>	m_isActive;
	std::vector<float> m_currentTime;
	std::vector<float> m_lifeTime;
};

}