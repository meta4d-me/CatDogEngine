#pragma once
#include "Base/Template.h"
#include "Core/StringCrc.h"
#include "Math/Vector.hpp"
#include "Scene/Types.h"

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

enum ParticleTypeVertexCount
{
    SpriteVertexCount = 4,
    RibbonVertetxCount = 2,
    TrackVertexCount = 3,
    RingVertexCount = 8,
    ModeVertexCountl
};

class Particle final
{
public:
	Particle() { Reset(); }
	Particle(const Particle&) = default;
	Particle& operator=(const Particle&) = default;
	Particle(Particle&&) = default;
	Particle& operator=(Particle&&) = default;
	~Particle() = default;

    cd::Vec3f& GetPos() { return m_particlePos; }
    void SetPos(cd::Vec3f pos) { m_particlePos = pos; }
    void SetSpeed(cd::Vec3f speed) { m_particleSpeed = speed; }
    void SetAcceleration(cd::Vec3f acceleration) { m_particleAcceleration = acceleration; }
    void SetColor(cd::Vec4f color) { m_particleColor = color; }
    
    void SetLifeTime(float lifeTime) { m_lifeTime = lifeTime; }
    void Active() { m_isActive = true; }
    bool isActive() const { return m_isActive; }

    void Reset();

    void Update(float deltaTime);

private:
    cd::Vec3f m_particlePos;
    cd::Vec3f m_particleSpeed;
    cd::Vec3f m_particleAcceleration;


    bool m_isActive;
    float m_currentTime;
    float m_lifeTime;

    cd::Color m_particleColor;
};

}