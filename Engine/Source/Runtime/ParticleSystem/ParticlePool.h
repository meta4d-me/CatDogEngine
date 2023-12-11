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
	ParticlePool() 
	{
		m_partcles = new Particle[m_maxParticleCount];
		m_freeParticleIndexes.clear();
		m_freeParticleIndexes.reserve(m_maxParticleCount);
	}
	ParticlePool(const ParticlePool&) = default;
	ParticlePool& operator=(const ParticlePool&) = default;
	ParticlePool(ParticlePool&&) = default;
	ParticlePool& operator=(ParticlePool&&) = default;
	~ParticlePool() { delete[]  m_partcles; }

	int AllocateParticleIndex();
	Particle& GetParticle(int index) { return m_partcles[index]; }
	int& GetParticleCount() { return m_currentActiveCount; }
	void Update(float deltaTime);

private:
	int m_maxParticleCount = 75;
	int m_currentActiveCount = 0;
	int m_currentParticleCount = 0;
	Particle* m_partcles;
	std::vector<int> m_freeParticleIndexes;
};

}