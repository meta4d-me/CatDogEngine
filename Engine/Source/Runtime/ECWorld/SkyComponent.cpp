#include "SkyComponent.h"

namespace engine
{

void SkyComponent::SetSkyType(SkyType type)
{
	m_type = type;
}

void SkyComponent::SetIrradianceTexturePath(std::string path)
{
	m_irradianceTexturePath = cd::MoveTemp(path);
}

void SkyComponent::SetRadianceTexturePath(std::string path)
{
	m_radianceTexturePath = cd::MoveTemp(path);
}

}