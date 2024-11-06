#pragma once

#include "Base/Template.h"
#include "Core/StringCrc.h"
#include "Math/Vector.hpp"

#include <vector>

#include "Particle.h"

namespace engine
{

class ParticlePool final
{
public:
	ParticlePool() = default;
	ParticlePool(const ParticlePool&) = default;
	ParticlePool& operator=(const ParticlePool&) = default;
	ParticlePool(ParticlePool&&) = default;
	ParticlePool& operator=(ParticlePool&&) = default;
	~ParticlePool() = default;

	int AllocateParticleIndex();
	Particle& GetParticle(int index) { return m_particles[index]; }
	int GetParticleActiveCount() { return m_currentActiveCount; }
	int& GetParticleMaxCount() { return m_maxParticleCount; }
	void SetParticleMaxCount(int count) { m_maxParticleCount = count; }

	void Update(float deltaTime);
	void AllParticlesReset();

private:
	int m_maxParticleCount = 75;
	int m_currentActiveCount = 0;
	int m_currentParticleCount = 0;
	std::vector<Particle> m_particles;
	std::vector<int> m_freeParticleIndexes;
};

}