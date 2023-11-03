#pragma once

#include "Base/Template.h"
#include "Core/StringCrc.h"
#include "Math/Vector.hpp"
#include "Math/Transform.hpp"
#include "ParticleSystem/ParticleSystem.h"

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

	cd::Vec3f& GetFVelocity() { return m_fill_velocity; }
	void SetFVelocity(cd::Vec3f fillnum) { m_fill_velocity = fillnum; }
	
	cd::Vec4f& GetFColor() { return m_fill_color; }
	void SetFColor(cd::Vec4f fillcolor) { m_fill_color = fillcolor; }

	uint16_t& GetParticleVBH(){ return m_particleVBH; }
	uint16_t& GetParticleIBH() { return m_particleIBH; }

	std::vector<std::byte> &GetVertexBuffer() { return m_particleVertexBuffer; }
	std::vector<std::byte> &GetIndexBuffer() { return m_particleIndexBuffer; }

	void Build();

	//void UpdateBuffer();

	void PaddingVertexBuffer();

	void PaddingIndexBuffer();

private:
	ParticleSystem			m_particleSystem;

	cd::Vec3f m_fill_velocity;
	cd::Vec4f m_fill_color{1.0f,1.0f,1.0f,1.0f};

	std::vector<std::byte> m_particleVertexBuffer;
	std::vector<std::byte> m_particleIndexBuffer;
	uint16_t m_particleVBH = UINT16_MAX;
	uint16_t m_particleIBH = UINT16_MAX;
};

}