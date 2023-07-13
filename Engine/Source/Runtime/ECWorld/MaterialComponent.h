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

	void Init(const engine::MaterialType* pMaterialType, const cd::Material* pMaterialData = nullptr);
	const engine::MaterialType* GetMaterialType() const { return m_pMaterialType; }

	void AddTextureBlob(cd::MaterialTextureType textureType, cd::TextureFormat textureFormat, cd::TextureMapMode uMapMode, cd::TextureMapMode vMapMode, TextureBlob textureBlob, uint32_t width, uint32_t height, uint32_t depth = 1);
	void AddTextureFileBlob(cd::MaterialTextureType textureType, const cd::Texture& texture, TextureBlob textureBlob);

	void SetUberShaderOption(StringCrc uberOption);
	StringCrc GetUberShaderOption() const;

	void SetAlbedoColor(cd::Vec3f color) { m_albedoColor = cd::MoveTemp(color); }
	cd::Vec3f& GetAlbedoColor() { return m_albedoColor; }
	const cd::Vec3f& GetAlbedoColor() const { return m_albedoColor; }

	void SetEmissiveColor(cd::Vec3f color) { m_emissiveColor = cd::MoveTemp(color); }
	cd::Vec3f& GetEmissiveColor() { return m_emissiveColor; }
	const cd::Vec3f& GetEmissiveColor() const { return m_emissiveColor; }

	void SetTwoSided(bool value) { m_twoSided = value; }
	bool& GetTwoSided() { return m_twoSided; }
	bool GetTwoSided() const { return m_twoSided; }

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

	cd::Vec3f m_albedoColor;
	cd::Vec3f m_emissiveColor;
	bool m_twoSided;

	// Output
	std::map<cd::MaterialTextureType, TextureInfo> m_textureResources;
};

}