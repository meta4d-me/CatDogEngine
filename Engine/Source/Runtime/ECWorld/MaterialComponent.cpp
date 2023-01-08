#include "MaterialComponent.h"

#include <cassert>

namespace engine
{

void MaterialComponent::SetTextureInfo(cd::MaterialTextureType textureType, uint8_t slot, uint16_t samplerHandle, uint16_t textureHandle)
{
	assert(!m_textureResources.contains(textureType));
	m_textureResources[textureType] = { .slot = slot, .samplerHandle = samplerHandle, .textureHandle = textureHandle };
}

std::optional<MaterialComponent::TextureInfo> MaterialComponent::GetTextureInfo(cd::MaterialTextureType textureType) const
{
	auto itTextureInfo = m_textureResources.find(textureType);
	if (itTextureInfo == m_textureResources.end())
	{
		return std::nullopt;
	}

	return itTextureInfo->second;
}

}