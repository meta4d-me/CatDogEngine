#pragma once

#include "Core/StringCrc.h"
#include "Scene/Material.h"
#include "Scene/MaterialTextureType.h"
#include "Scene/Texture.h"

#include <cstdint>
#include <map>
#include <optional>
#include <vector>

namespace bgfx
{

struct Memory;

}

namespace cd
{

class Material;

}

namespace engine
{

class MaterialType;

class MaterialComponent final
{
public:
	static constexpr StringCrc GetClassName()
	{
		constexpr StringCrc className("MaterialComponent");
		return className;
	}

	using TextureBlob = std::vector<std::byte>;
	struct TextureInfo
	{
		const bgfx::Memory* data;
		uint64_t flag;
		uint32_t width;
		uint32_t height;
		uint32_t depth;
		cd::TextureFormat format;
		cd::Vec2f uvOffset;
		cd::Vec2f uvScale;
		uint16_t samplerHandle;
		uint16_t textureHandle;
		uint8_t slot;
		uint8_t mipCount;
	};

public:
	MaterialComponent() = default;
	MaterialComponent(const MaterialComponent&) = default;
	MaterialComponent& operator=(const MaterialComponent&) = default;
	MaterialComponent(MaterialComponent&&) = default;
	MaterialComponent& operator=(MaterialComponent&&) = default;
	~MaterialComponent() = default;

	void SetMaterialData(const cd::Material* pMaterialData) { m_pMaterialData = pMaterialData; }
	void SetMaterialType(const engine::MaterialType* pMaterialType) { m_pMaterialType = pMaterialType; }
	const engine::MaterialType* GetMaterialType() const { return m_pMaterialType; }

	void AddTextureBlob(cd::MaterialTextureType textureType, cd::TextureFormat textureFormat, cd::TextureMapMode uMapMode, cd::TextureMapMode vMapMode, TextureBlob textureBlob, uint32_t width, uint32_t height, uint32_t depth = 1);
	void AddTextureFileBlob(cd::MaterialTextureType textureType, const cd::Texture& texture, TextureBlob textureBlob);

	void SetUberShaderOption(StringCrc uberOption);
	StringCrc GetUberShaderOption() const;
	uint16_t GetShadingProgram() const;

	std::optional<const TextureInfo> GetTextureInfo(cd::MaterialTextureType textureType) const;
	const std::map<cd::MaterialTextureType, TextureInfo>& GetTextureResources() const { return m_textureResources; }

	void Reset();
	void Build();

private:
	// Input
	const cd::Material* m_pMaterialData = nullptr;
	const engine::MaterialType* m_pMaterialType = nullptr;
	StringCrc m_uberShaderOption;

	// Output
	std::map<cd::MaterialTextureType, TextureInfo> m_textureResources;
};

}