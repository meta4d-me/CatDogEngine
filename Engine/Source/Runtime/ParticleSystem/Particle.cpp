#include "Particle.h"

namespace engine
{

void Particle::Reset()
{
    m_particlePos = cd::Vec3f::Zero();
    m_particleSpeed = cd::Vec3f::Zero();
    m_particleSpeed = cd::Vec3f::Zero();

    m_isActive = false;
    m_currentTime = 0.0f;
    m_lifeTime = 6.0f;

    m_particleColor = cd::Vec4f::One();
}

void Particle::Update(float deltaTime)
{
    if (m_currentTime >= m_lifeTime)
    {
        m_isActive = false;
        return;
    }

    m_particlePos.x() = m_particlePos.x() + m_particleSpeed.x() * deltaTime + 0.5f * m_particleAcceleration.x() * deltaTime * deltaTime;
    m_particlePos.y() = m_particlePos.y() + m_particleSpeed.y() * deltaTime + 0.5f * m_particleAcceleration.y() * deltaTime * deltaTime;
    m_particlePos.z() = m_particlePos.z() + m_particleSpeed.z() * deltaTime + 0.5f * m_particleAcceleration.z() * deltaTime * deltaTime;

    
    m_particleSpeed.x() += m_particleAcceleration.x() * deltaTime;
    m_particleSpeed.y() += m_particleAcceleration.y() * deltaTime;
    m_particleSpeed.z() += m_particleAcceleration.z() * deltaTime;

    if (m_rotationForceField)
    {
        cd::Vec3f zForward{0.0f, 0.0f, 1.0f};
        cd::Vec3f CentripetalV = zForward.Cross(m_particleSpeed);
        m_particleAcceleration = CentripetalV*5;
    }

    m_currentTime += deltaTime;
}

}