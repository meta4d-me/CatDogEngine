#pragma once
#include "Core/StringCrc.h"
#include <vector>
#include <bgfx/bgfx.h>

namespace engine
{
enum ElementType
{
    None,
    Float,
    Double,
    UChar
};

class GaussianRenderComponent final
{
public:
	static constexpr StringCrc GetClassName()
	{
		constexpr StringCrc className("GaussianRenderComponent");
		return className;
	}

public:
	GaussianRenderComponent() = default;
	GaussianRenderComponent(const GaussianRenderComponent&) = default;
	GaussianRenderComponent& operator=(const GaussianRenderComponent&) = default;
	GaussianRenderComponent(GaussianRenderComponent&&) = default;
	GaussianRenderComponent& operator=(GaussianRenderComponent&&) = default;
	~GaussianRenderComponent() = default;

	void Build();

	uint16_t& GetVertexBufferHandle() { return m_vertexBufferHandle; }
	uint16_t& GetIndexBufferHandle() { return m_indexBufferHandle; }

	std::vector<std::byte>& GetVertexBuffer() { return m_vertexBuffer; }
	std::vector<std::byte>& GetIndexBuffer() { return m_indexBuffer; }

	
	std::vector<std::byte>& GetGaussianBuffer() { return m_gausianAttributesBuffer; }

	//ply
	void SetPlyData(std::vector<std::byte>& buffer) { m_readBuffer = buffer; }

	void ProcessingPlyBuffer();

	void GenerateTexture();

	uint32_t& GetVertexCount() { return m_vertextCount; }

private:
	std::vector<std::byte> m_vertexBuffer;
	std::vector<std::byte> m_indexBuffer;
	uint16_t m_vertexBufferHandle = UINT16_MAX;
	uint16_t m_indexBufferHandle = UINT16_MAX;

	//ply
	std::vector<std::byte> m_readBuffer;

	std::vector<float> m_sizeList;
	std::vector<uint32_t> m_sizeIndex;
	uint32_t m_vertextCount;
	//gaussian
	std::vector<std::byte> m_gausianAttributesBuffer;

	//texture
	std::vector<std::byte> m_textureBuffer;
	bgfx::TextureHandle m_textureHandle;
};

}