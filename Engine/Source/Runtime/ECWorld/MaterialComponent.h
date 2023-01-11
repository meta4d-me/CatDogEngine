#pragma once

#include "Core/StringCrc.h"
#include "Material/MaterialType.h"
#include "Scene/Material.h"
#include "Scene/MaterialTextureType.h"
#include "Scene/Texture.h"

#include <cstdint>
#include <map>
#include <optional>
#include <vector>

namespace cd
{

class Material;

}

namespace engine
{

class MaterialType;

class MaterialComponent
{
public:
	static constexpr StringCrc GetClassName()
	{
		constexpr StringCrc className("MaterialComponent");
		return className;
	}

	struct TextureInfo
	{
		uint8_t slot;
		uint16_t samplerHandle;
		uint16_t textureHandle;
		uint8_t padding;
	};

public:
	MaterialComponent() = default;
	MaterialComponent(const MaterialComponent&) = default;
	MaterialComponent& operator=(const MaterialComponent&) = default;
	MaterialComponent(MaterialComponent&&) = default;
	MaterialComponent& operator=(MaterialComponent&&) = default;
	~MaterialComponent() = default;

	void SetMaterialData(const cd::Material* pMaterialData) { m_pMaterialData = pMaterialData; }
	void AddTextureData(const cd::Texture* pTextureData) { m_pTextureDatas.push_back(pTextureData); }

	void SetShadingProgram(uint16_t shadingProgram) { m_shadingProgram = shadingProgram; }
	uint16_t GetShadingProgram() const { return m_shadingProgram; }

	void SetTextureInfo(cd::MaterialTextureType textureType, uint8_t slot, uint16_t samplerHandle, uint16_t textureHandle);
	std::optional<TextureInfo> GetTextureInfo(cd::MaterialTextureType textureType) const;

	void Reset();
	void Build();

private:
	void BuildTexture(const char* pTextureFilePath);
	void BuildShader(const char* pVSFilePath, const char* pFSFilePath);

private:
	// Input
	const cd::Material* m_pMaterialData = nullptr;
	std::vector<const cd::Texture*> m_pTextureDatas;
	uint16_t m_shadingProgram = INT16_MAX;

	// Output
	std::map<cd::MaterialTextureType, TextureInfo> m_textureResources;
};

}