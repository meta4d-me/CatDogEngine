#pragma once

#include "Core/StringCrc.h"
#include "Math/Vector.hpp"
#include "Scene/MaterialTextureType.h"

#include <cstdint>
#include <map>
#include <optional>

namespace cd
{

class Material;

}

namespace engine
{

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

	void SetShadingProgram(uint16_t shadingProgram) { m_shadingProgram = shadingProgram; }
	uint16_t GetShadingProgram() const { return m_shadingProgram; }

	void SetTextureInfo(cd::MaterialTextureType textureType, uint8_t slot, uint16_t samplerHandle, uint16_t textureHandle);
	std::optional<TextureInfo> GetTextureInfo(cd::MaterialTextureType textureType) const;

private:
	// Input
	const cd::Material* m_pMaterialData = nullptr;
	uint16_t m_shadingProgram = INT16_MAX;

	// TODO : Build textures.

	// Output
	std::map<cd::MaterialTextureType, TextureInfo> m_textureResources;
};

}