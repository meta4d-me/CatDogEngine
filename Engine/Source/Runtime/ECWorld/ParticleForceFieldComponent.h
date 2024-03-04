#pragma once

#include "Base/Template.h"
#include "Core/StringCrc.h"
#include "Math/Vector.hpp"

#include <vector>
namespace engine
{
enum ForceFieldType
{
	ForceFieldSphere,
	ForceFieldHemisphere,
	ForceFieldCylinder,
	ForceFieldBox
};

class ParticleForceFieldComponent final
{
public:
	static constexpr StringCrc GetClassName()
	{
		constexpr StringCrc className("ParticleForceFieldComponent");
		return className;
	}

	ParticleForceFieldComponent() = default;
	ParticleForceFieldComponent(const ParticleForceFieldComponent&) = default;
	ParticleForceFieldComponent& operator=(const ParticleForceFieldComponent&) = default;
	ParticleForceFieldComponent(ParticleForceFieldComponent&&) = default;
	ParticleForceFieldComponent& operator=(ParticleForceFieldComponent&&) = default;
	~ParticleForceFieldComponent() = default;

	cd::Vec3f& GetForceFieldRange() { return m_forcefieldRange; }
	void SetForceFieldRange(cd::Vec3f range) { m_forcefieldRange = range; }

	bool& GetRotationForce() { return m_rotationForce; }
	void SetRotationForce(bool force) { m_rotationForce = force; }

	uint16_t& GetVertexBufferHandle() { return m_vertexBufferHandle; }
	uint16_t& GetIndexBufferHandle() { return m_indexBufferHandle; }

	std::vector<std::byte>& GetVertexBuffer() { return m_vertexBuffer; }
	std::vector<std::byte>& GetIndexBuffer() { return m_indexBuffer; }

	void Build();

	bool IfWithinTheRange(ForceFieldType type, float m_startRange, float m_endRange);

private:
	ForceFieldType m_foceFieldType = ForceFieldType::ForceFieldBox;
	//size
	cd::Vec3f m_forcefieldRange{3.0f, 3.0f,3.0f};

	cd::Vec3f m_direction;

	float m_gravityStrength;
	float m_gravityFocus;

	bool m_rotationForce = false;
	float m_rotationSpeed;
	float m_rotationAttraction;
	float m_rotationRandomness;

	float m_dragStrength;
	bool m_dragMultiplyBySize;
	bool m_dragMultiplyByVelocity;

	//TODO: Vector Field

	std::vector<std::byte> m_vertexBuffer;
	std::vector<std::byte> m_indexBuffer;
	uint16_t m_vertexBufferHandle = UINT16_MAX;
	uint16_t m_indexBufferHandle = UINT16_MAX;


};

}