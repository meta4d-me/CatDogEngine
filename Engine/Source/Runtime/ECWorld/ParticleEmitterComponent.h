#pragma once

#include "Base/Template.h"
#include "Core/StringCrc.h"
#include "Math/Vector.hpp"
#include "Math/Transform.hpp"
#include "ParticleSystem/ParticleSystem.h"
#include "Scene/VertexFormat.h"

namespace engine
{

class ParticleEmitterComponent final
{
public:
	static constexpr StringCrc GetClassName()
	{
		constexpr StringCrc className("ParticleEmitterComponent");
		return className;
	}

	ParticleEmitterComponent() = default;
	ParticleEmitterComponent(const ParticleEmitterComponent&) = default;
	ParticleEmitterComponent& operator=(const ParticleEmitterComponent&) = default;
	ParticleEmitterComponent(ParticleEmitterComponent&&) = default;
	ParticleEmitterComponent& operator=(ParticleEmitterComponent&&) = default;
	~ParticleEmitterComponent() = default;

	engine::ParticleSystem &GetParticleSystem() { return m_particleSystem; }

	int& GetParticleNum() { return m_particleNum; }
	void SetParticleNum(int num) { m_particleNum = num; }

	engine::ParticleType& GetEmitterParticleType() { return m_emitterparticletype; }
	void SetEmitterParticleType(engine::ParticleType type) { m_emitterparticletype = type; }

	bool& GetRandomVelocityState() { return m_randomVelocityState; }
	void SetRandomVelocityState(bool state) { m_randomVelocityState = state; }

	cd::Vec3f& GetRandomVelocity() { return m_randomVelocity; }
	void SetRandomVelocity(cd::Vec3f velocity) { m_randomVelocity = velocity; }

	cd::Vec3f& GetEmitterVelocity() { return m_emitter_velocity; }
	void SetEmitterVelocity(cd::Vec3f velocity) { m_emitter_velocity = velocity; }
	
	cd::Vec4f& GetEmitterColor() { return m_emitter_color; }
	void SetEmitterColor(cd::Vec4f fillcolor) { m_emitter_color = fillcolor; }

	uint16_t& GetParticleVertexBufferHandle(){ return m_particleVertexBufferHandle; }
	uint16_t& GetParticleIndexBufferHandle() { return m_particleIndexBufferHandle; }

	std::vector<std::byte> &GetVertexBuffer() { return m_particleVertexBuffer; }
	std::vector<std::byte> &GetIndexBuffer() { return m_particleIndexBuffer; }

	void Build();

	void SetRequiredVertexFormat(const cd::VertexFormat* pVertexFormat) { m_pRequiredVertexFormat = pVertexFormat; }
	//void UpdateBuffer();

	void PaddingVertexBuffer();

	void PaddingIndexBuffer();

private:
	ParticleSystem m_particleSystem;

	engine::ParticleType m_emitterparticletype = engine::ParticleType::Sprite;

	int m_particleNum = 10;
	bool m_randomVelocityState;
	cd::Vec3f m_randomVelocity;
	cd::Vec3f m_emitter_velocity;
	cd::Vec4f m_emitter_color{1.0f,1.0f,1.0f,1.0f};

	const cd::VertexFormat* m_pRequiredVertexFormat = nullptr;
	std::vector<std::byte> m_particleVertexBuffer;
	std::vector<std::byte> m_particleIndexBuffer;
	uint16_t m_particleVertexBufferHandle = UINT16_MAX;
	uint16_t m_particleIndexBufferHandle = UINT16_MAX;
};

}