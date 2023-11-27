#pragma once

#include "Base/Template.h"
#include "Core/StringCrc.h"
#include "ECWorld/TransformComponent.h"
#include "Math/Vector.hpp"

#include <vector>

namespace engine
{

enum ParticleType
{
	Sprite,
	Ribbon,
	Track,
	Ring,
	Model
};

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
	void SetMaxCount(int num);

	int& GetParticleActiveCount() { return m_currentActiveCount; }

	engine::ParticleType& GetType() { return m_particletype; }
	void SetType(engine::ParticleType type) { m_particletype = type; }

	bool& GetRandomState() { return m_RandomState; }
	void SetRandomState(bool state) { m_RandomState = state; }

	cd::Vec3f& GetTwoSideVelocity() { return m_twoSideVelocity; }
	void SetTwoSideVelocity(cd::Vec3f velocity) { m_twoSideVelocity = velocity; }

	cd::Vec3f& GetPos(int index) { return m_pos[index]; }
	void SetPos(cd::Vec3f pos) { m_pos[m_particleIndex] = pos; }

	cd::Vec3f& GetRotation(int index) { return m_rotation[index]; }
	void SetRotation(cd::Vec3f rotate) { m_rotation[m_particleIndex] = rotate; }

	cd::Vec3f& GetScale(int index) { return m_scale[index]; }
	void SetScale(cd::Vec3f scale) { m_scale[m_particleIndex] = scale; }

	cd::Vec3f& GetVelocity() { return m_velocity[m_particleIndex]; }
	void SetVelocity(cd::Vec3f velocity) { m_velocity[m_particleIndex] = velocity; }

	cd::Vec3f& GetVelocityXYZ() { return m_velocityXYZ[m_particleIndex]; }
	void SetVelocityXYZ(cd::Vec3f velocity) { m_velocity[m_particleIndex] = velocity; }

	//cd::Quaternion& GetEmiterDirection( ) { return m_emiterDirection[m_particleIndex]; }
	//void SetEmiterDirection(cd::Quaternion direction) { m_emiterDirection[m_particleIndex] = direction; }

	cd::Vec3f& GetAcceleration(int index) { return m_acceleration[index]; }
	void SetAcceleration(cd::Vec3f acceleration) { m_acceleration[m_particleIndex] = acceleration; }

	cd::Vec4f& GetColor(int index) { return m_color[index]; }
	void SetColor(cd::Vec4f color) { m_color[m_particleIndex] = color; }

	cd::UV& GetTexture_uv(int index) { return m_texture_uv[index]; }

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
	int m_particleMaxCount = 1200;
	int m_currentActiveCount = 0;

	engine::ParticleType m_particletype;
	bool m_RandomState;
	cd::Vec3f m_twoSideVelocity;
	std::vector<int> m_freeParticleIndex;
	std::vector<cd::Vec3f> m_pos;
	std::vector<cd::Vec3f> m_rotation;
	std::vector<cd::Vec3f> m_scale;

	std::vector<cd::Vec3f> m_velocity;
	std::vector<cd::Vec3f> m_velocityXYZ;
	//std::vector<cd::Quaternion> m_emiterDirection;
	std::vector<cd::Vec3f> m_acceleration;
	std::vector<cd::Vec4f> m_color;
	std::vector<cd::UV> m_texture_uv;

	std::vector<bool> m_isActive;
	std::vector<float> m_currentTime;
	std::vector<float> m_lifeTime;
};

}