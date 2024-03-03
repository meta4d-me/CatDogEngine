#pragma once

#include "Base/Template.h"
#include "Core/StringCrc.h"
#include "Math/Vector.hpp"
#include "Scene/Types.h"

#include <vector>

namespace engine
{

enum class ParticleType
{
    Sprite,
    Ribbon,
    Track,
    Ring,
    Model
};

class Particle final
{
public:
    template<ParticleType PT>
    static constexpr int GetMeshVertexCount()
    {
        if constexpr (ParticleType::Sprite == PT)
        {
            return 4;
        }
        else if constexpr (ParticleType::Ribbon == PT)
        {
            return 2;
        }
        else if constexpr (ParticleType::Track == PT)
        {
            return 3;
        }
        else if constexpr (ParticleType::Ring == PT)
        {
            return 8;
        }

        return 3;
    }

public:
	Particle() { Reset(); }
	Particle(const Particle&) = default;
	Particle& operator=(const Particle&) = default;
	Particle(Particle&&) = default;
	Particle& operator=(Particle&&) = default;
	~Particle() = default;

    cd::Vec3f& GetPos() { return m_particlePos; }
    void SetPos(cd::Vec3f pos) { m_particlePos = pos; }

    cd::Vec3f GetSpeed() { return m_particleSpeed; }
    void SetSpeed(cd::Vec3f speed) { m_particleSpeed = speed; }

    void SetAcceleration(cd::Vec3f acceleration) { m_particleAcceleration = acceleration; }
    void SetColor(cd::Vec4f color) { m_particleColor = color; }
    
    void SetLifeTime(float lifeTime) { m_lifeTime = lifeTime; }

    void SetRotationForceField(bool value) { m_rotationForceField = value; }
    void SetRotationForceFieldRange(cd::Vec3f range) { m_rotationForceFieldRange = range; }

    void Active() { m_isActive = true; }
    bool isActive() const { return m_isActive; }

    void Reset();

    void Update(float deltaTime);

private:
    cd::Vec3f m_particlePos;
    cd::Vec3f m_particleSpeed;
    cd::Vec3f m_particleAcceleration;

    bool m_rotationForceField = false;
    cd::Vec3f m_rotationForceFieldRange;

    bool m_isActive;
    float m_currentTime;
    float m_lifeTime;

    cd::Color m_particleColor;
};

}