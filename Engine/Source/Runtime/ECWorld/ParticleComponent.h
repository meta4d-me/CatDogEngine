#pragma once

#include "Base/Template.h"
#include "Core/StringCrc.h"
#include "Math/Vector.hpp"
#include "Particle/EmitterDefinition.h"

namespace engine
{

class ParticleComponent final
{
public:
	static constexpr StringCrc GetClassName()
	{
		constexpr StringCrc className("ParticleComponent");
		return className;
	}

	ParticleComponent() = default;
	ParticleComponent(const ParticleComponent&) = default;
	ParticleComponent& operator=(const ParticleComponent&) = default;
	ParticleComponent(ParticleComponent&&) = default;
	ParticleComponent& operator=(ParticleComponent&&) = default;
	~ParticleComponent() = default;

    void SetShape(EmitterShape::Enum shape) { m_shape = shape; }
    EmitterShape::Enum& GetShape() { return m_shape; }
    const EmitterShape::Enum& GetShape() const { return m_shape; }

    void SetDirection(EmitterDirection::Enum direction) { m_direction = direction; }
    EmitterDirection::Enum& GetDirection() { return m_direction; }
    const EmitterDirection::Enum& GetDirection() const { return m_direction; }

    void SetNeedRecreate(bool needRecreate) { m_needRecreate = needRecreate; }
    bool& GetNeedRecreate() { return m_needRecreate; }
    const bool& GetNeedRecreate() const { return m_needRecreate; }

    void SetParticlesPerSecond(uint32_t particlesPerSecond) { m_particlesPerSecond = particlesPerSecond; }
    uint32_t& GetParticlesPerSecond() { return m_particlesPerSecond; }
    const uint32_t& GetParticlesPerSecond() const { return m_particlesPerSecond; }

    void SetGravityScale(float gravityScale) { m_gravityScale = gravityScale; }
    float& GetGravityScale() { return m_gravityScale; }
    const float& GetGravityScale() const { return m_gravityScale; }

    void SetLifeSpan(float lifeSpan, size_t index) { m_lifeSpan[index] = lifeSpan; }
    float& GetLifeSpan(size_t index) { return  m_lifeSpan[index]; }
    const float& GetLifeSpan(size_t index) const { return  m_lifeSpan[index]; }

    void SetOffsetStart(float offsetStart, size_t index) { m_offsetStart[index] = offsetStart; }
    float& GetOffsetStart(size_t index) { return  m_offsetStart[index]; }
    const float& GetOffsetStart(size_t index) const { return  m_offsetStart[index]; }

    void SetOffsetEnd(float offsetEnd, size_t index) { m_offsetEnd[index] = offsetEnd; }
    float& GetOffsetEnd(size_t index) { return  m_offsetEnd[index]; }
    const float& GetOffsetEnd(size_t index) const { return  m_offsetEnd[index]; }

    void SetScaleStart(float scaleStart, size_t index) { m_scaleStart[index] = scaleStart; }
    float& GetScaleStart(size_t index) { return  m_scaleStart[index]; }
    const float& GetScaleStart(size_t index) const { return  m_scaleStart[index]; }

    void SetScaleEnd(float scaleEnd, size_t index) { m_scaleEnd[index] = scaleEnd; }
    float& GetScaleEnd(size_t index) { return  m_scaleEnd[index]; }
    const float& GetScaleEnd(size_t index) const { return  m_scaleEnd[index]; }

    void SetRgba(uint32_t rgba, size_t index) { m_rgba[index] = rgba; }
    uint32_t& GetRgba(size_t index) { return  m_rgba[index]; }
    const uint32_t& GetRgba(size_t index)const { return  m_rgba[index]; }

private:
	EmitterShape::Enum m_shape;
	EmitterDirection::Enum m_direction;
	bool m_needRecreate = false;

	uint32_t m_particlesPerSecond;
	float m_gravityScale;
	float m_lifeSpan[2];

	float m_offsetStart[2];
	float m_offsetEnd[2];
	float m_scaleStart[2];
	float m_scaleEnd[2];

	uint32_t m_rgba[5];
};

}