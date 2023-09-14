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

	engine::ParticleSystem GetParticleSystem() { return m_particleSystem; }
	void SetParticleSystem(engine::ParticleSystem& system) { m_particleSystem = system; }

	uint16_t& GetParticleVBH(){ return m_particleVBH; }
	void setParticleVBH(uint16_t vbh) { m_particleVBH = vbh; }

	uint16_t& GetParticleIBH() { return m_particleIBH; }
	void SetParticleIBH(uint16_t ibh) { m_particleIBH = ibh; }

	std::vector<std::byte> &GetVertexBuffer() { return m_particleVertexBuffer; }
	std::vector<std::byte> &GetIndexBuffer() { return m_particleIndexBuffer; }

	void Build();

private:
	ParticleSystem			m_particleSystem;
	std::vector<std::byte> m_particleVertexBuffer;
	std::vector<std::byte> m_particleIndexBuffer;
	uint16_t m_particleVBH = UINT16_MAX;
	uint16_t m_particleIBH = UINT16_MAX;
};

}