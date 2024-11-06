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

class ParticleRibbonComponent final
{
public:
	static constexpr StringCrc GetClassName()
	{
		constexpr StringCrc className("ParticleRibbonComponent");
		return className;
	}

	ParticleRibbonComponent() = default;
	ParticleRibbonComponent(const ParticleRibbonComponent&) = default;
	ParticleRibbonComponent& operator=(const ParticleRibbonComponent&) = default;
	ParticleRibbonComponent(ParticleRibbonComponent&&) = default;
	ParticleRibbonComponent& operator=(ParticleRibbonComponent&&) = default;
	~ParticleRibbonComponent() = default;

	void PaddingRibbonVertexBuffer();
	void PaddingRibbonIndexBuffer();

	void Build();

	uint16_t& GetRibbonParticlePrePosVertexBufferHandle() { return m_ribbonParticlePrePosVertexBufferHandle; }
	uint16_t& GetRibbonParticleRemainVertexBufferHandle() { return m_ribbonParticleRemainVertexBufferHandle; }
	uint16_t& GetRibbonParticleIndexBufferHandle() { return m_ribbonParticleIndexBufferHandle; }

	std::vector<std::byte>& GetRibbonPrePosVertexBuffer() { return m_ribbonParticlePrePosVertexBuffer; }
	std::vector<std::byte>& GetRibbonRemainVertexBuffer() { return m_ribbonParticleRemainVertexBuffer; }
	std::vector<std::byte>& GetRibbonIndexBuffer() { return m_ribbonParticleIndexBuffer; }

private:
	//particle vertex/index
	struct VertexData
	{
		cd::Vec3f pos;
		cd::Vec4f color;
		cd::UV     uv;
	};

	std::vector<std::byte> m_ribbonParticlePrePosVertexBuffer;
	std::vector<std::byte> m_ribbonParticleRemainVertexBuffer;
	std::vector<std::byte> m_ribbonParticleIndexBuffer;
	uint16_t m_ribbonParticlePrePosVertexBufferHandle = UINT16_MAX;
	uint16_t m_ribbonParticleRemainVertexBufferHandle = UINT16_MAX;
	uint16_t m_ribbonParticleIndexBufferHandle = UINT16_MAX;
};

}