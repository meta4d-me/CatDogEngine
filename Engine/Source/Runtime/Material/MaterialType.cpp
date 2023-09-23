#include "MaterialType.h"

#include "Math/Vector.hpp"

namespace engine
{

void MaterialType::AddOptionalTextureType(cd::MaterialTextureType textureType, uint8_t slot)
{
	m_optionalTextureTypes.insert(textureType);
	m_textureTypeSlots[textureType] = slot;
}

std::optional<uint8_t> MaterialType::GetTextureSlot(cd::MaterialTextureType textureType) const
{
	auto itTexture = m_textureTypeSlots.find(textureType);
	if (itTexture == m_textureTypeSlots.end())
	{
		return std::nullopt;
	}

	return itTexture->second;
}

}