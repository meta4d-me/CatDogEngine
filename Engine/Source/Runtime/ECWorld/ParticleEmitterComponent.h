#pragma once

#include "Base/Template.h"
#include "Core/StringCrc.h"
#include "Math/Vector.hpp"
#include "Math/Transform.hpp"
#include "Material/ShaderSchema.h"
#include "Material/MaterialType.h"
#include "ParticleSystem/ParticlePool.h"
#include "Scene/Mesh.h"
#include "Scene/Types.h"
#include "Scene/VertexFormat.h"

namespace engine
{
enum class ParticleRenderMode
{
	Billboard,
	Mesh,
};

enum class ParticleEmitterShape
{
	Sphere,
	Hemisphere,
	Box
};

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

	ParticlePool& GetParticlePool() { return m_particlePool; }

	int& GetSpawnCount() { return m_spawnCount; }
	void SetSpawnCount(int count) { m_spawnCount = count; }

	ParticleEmitterShape& GetEmitterShape() { return m_emitterShape; }
	void SetEmitterShape(ParticleEmitterShape shape) { m_emitterShape = shape; }

	cd::Vec3f& GetEmitterShapeRange() { return m_emitterShapeRange; }
	void SetEmitterShapeRange(cd::Vec3f range) { m_emitterShapeRange = range; }

	bool& GetInstanceState() { return m_useInstance; }
	void SetInstanceState(bool state) { m_useInstance = state; }

	const engine::MaterialType* GetMaterialType() const { return m_pParticleMaterialType; }
	void SetMaterialType(const engine::MaterialType* pMaterialType) { m_pParticleMaterialType = pMaterialType; }

	ParticleRenderMode& GetRenderMode() { return m_renderMode; }
	void SetRenderMode(engine::ParticleRenderMode mode) { m_renderMode = mode; }

	ParticleType& GetEmitterParticleType() { return m_emitterParticleType; }
	void SetEmitterParticleType(engine::ParticleType type) { m_emitterParticleType = type; }

	//random
	bool& GetRandomPosState() { return m_randomPosState; }
	cd::Vec3f& GetRandormPos() { return m_randomPos; }
	void SetRandomPos(cd::Vec3f randomPos) { m_randomPos = randomPos; }

	bool& GetRandomVelocityState() { return m_randomVelocityState; }
	cd::Vec3f& GetRandomVelocity() { return m_randomVelocity; }
	void SetRandomVelocity(cd::Vec3f randomVelocity) { m_randomVelocity = randomVelocity; }

	//particle data
	cd::Vec3f& GetEmitterVelocity() { return m_emitterVelocity; }
	void SetEmitterVelocity(cd::Vec3f velocity) { m_emitterVelocity = velocity; }

	cd::Vec3f& GetEmitterAcceleration() { return m_emitterAcceleration; }
	void SetEmitterAcceleration(cd::Vec3f accleration) { m_emitterAcceleration = accleration; }

	cd::Vec4f& GetEmitterColor() { return m_emitterColor; }
	void SetEmitterColor(cd::Vec4f fillcolor) { m_emitterColor = fillcolor; }

	float& GetLifeTime() { return m_emitterLifeTime; }
	void SetEmitterLifeTime(float lifetime) { m_emitterLifeTime = lifetime; }

	uint16_t& GetParticleVertexBufferHandle() { return m_particleVertexBufferHandle; }
	uint16_t& GetParticleIndexBufferHandle() { return m_particleIndexBufferHandle; }

	std::vector<std::byte>& GetVertexBuffer() { return m_particleVertexBuffer; }
	std::vector<std::byte>& GetIndexBuffer() { return m_particleIndexBuffer; }

	uint16_t& GetEmitterShapeVertexBufferHandle() { return m_emitterShapeVertexBufferHandle; }
	uint16_t& GetEmitterShapeIndexBufferHandle() { return m_emitterShapeIndexBufferHandle; }

	std::vector<std::byte>& GetEmitterShapeVertexBuffer() { return m_emitterShapeVertexBuffer; }
	std::vector<std::byte>& GetEmitterShapeIndexBuffer() { return m_emitterShapeIndexBuffer; }

	const cd::Mesh* GetMeshData() const { return m_pMeshData; }
	void SetMeshData(const cd::Mesh* pMeshData) { m_pMeshData = pMeshData; }

	void Build();

	void SetRequiredVertexFormat(const cd::VertexFormat* pVertexFormat) { m_pRequiredVertexFormat = pVertexFormat; }

	//void UpdateBuffer();
	void PaddingVertexBuffer();
	void PaddingIndexBuffer();

	void BuildParticleShape();
	void RePaddingShapeBuffer();

	// Uber shader data.
	const std::string& GetShaderProgramName() const;
	void ActivateShaderFeature(ShaderFeature feature);
	void DeactivateShaderFeature(ShaderFeature feature);
	void SetShaderFeatures(std::set<ShaderFeature> options) { m_shaderFeatures = cd::MoveTemp(m_shaderFeatures); }
	std::set<ShaderFeature>& GetShaderFeatures() { return m_shaderFeatures; }
	const std::set<ShaderFeature>& GetShaderFeatures() const { return m_shaderFeatures; }
	const std::string& GetFeaturesCombine();

private:
	//ParticleSystem m_particleSystem;
	ParticlePool m_particlePool;

	engine::ParticleType m_emitterParticleType;

	//emitter  data
	int m_spawnCount = 75;
	cd::Vec3f m_emitterVelocity {20.0f, 20.0f, 0.0f};
	cd::Vec3f m_emitterAcceleration;
	cd::Vec4f m_emitterColor = cd::Vec4f::One();
	float m_emitterLifeTime = 6.0f;

	// random emitter data
	bool m_randomPosState;
	cd::Vec3f m_randomPos;
	bool m_randomVelocityState;
	cd::Vec3f m_randomVelocity;

	//instancing
	bool m_useInstance = false;

	//Uber shader
	const engine::MaterialType* m_pParticleMaterialType = nullptr;
	bool m_isShaderFeatureDirty = false;
	std::set<ShaderFeature> m_shaderFeatures;
	std::string m_featureCombine;

	//render mode  mesh/billboard/ribbon
	ParticleRenderMode m_renderMode = ParticleRenderMode::Mesh;
	const cd::Mesh* m_pMeshData = nullptr;

	//particle vertex/index
	struct VertexData
	{
		cd::Vec3f pos;
		cd::Vec4f color;
		cd::UV     uv;
	};
	const cd::VertexFormat* m_pRequiredVertexFormat = nullptr;
	std::vector<std::byte> m_particleVertexBuffer;
	std::vector<std::byte> m_particleIndexBuffer;
	uint16_t m_particleVertexBufferHandle = UINT16_MAX;
	uint16_t m_particleIndexBufferHandle = UINT16_MAX;

	//emitter shape vertex/index
	ParticleEmitterShape m_emitterShape = ParticleEmitterShape::Box;
	cd::Vec3f m_emitterShapeRange {10.0f ,5.0f ,10.0f};
	std::vector<std::byte> m_emitterShapeVertexBuffer;
	std::vector<std::byte> m_emitterShapeIndexBuffer;
	uint16_t m_emitterShapeVertexBufferHandle = UINT16_MAX;
	uint16_t m_emitterShapeIndexBufferHandle = UINT16_MAX;
};

}