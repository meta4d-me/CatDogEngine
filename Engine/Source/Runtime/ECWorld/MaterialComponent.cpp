#include "MaterialComponent.h"

#include <cassert>

namespace engine
{

void MaterialComponent::SetTextureInfo(cd::MaterialTextureType textureType, uint8_t slot, uint16_t samplerHandle, uint16_t textureHandle)
{
	assert(!m_textureResources.contains(textureType));
	m_textureResources[textureType] = { .slot = slot, .samplerHandle = samplerHandle, .textureHandle = textureHandle };
}

const MaterialComponent::TextureInfo& MaterialComponent::GetTextureInfo(cd::MaterialTextureType textureType) const
{
	auto itTextureInfo = m_textureResources.find(textureType);

	if (itTextureInfo == m_textureResources.end())
	{
		assert("Failed to get texture resource.");
		static TextureInfo dummy;
		return dummy;
	}

	return itTextureInfo->second;
}

}