#include "SkyComponent.h"

namespace engine
{

void SkyComponent::SetSkyType(SkyType type)
{
	if (m_type == type)
	{
		return;
	}

	m_type = type;
	static std::string preRadPath = m_radianceTexturePath;
	if (SkyType::None == m_type)
	{
		preRadPath = cd::MoveTemp(m_radianceTexturePath);
		m_radianceTexturePath = m_pureGrayTexturePath;
	}
	else if(preRadPath != m_radianceTexturePath)
	{
		m_radianceTexturePath = preRadPath;
	} 
}

void SkyComponent::SetSunDirection(cd::Direction dir)
{
	m_sunDirection = cd::MoveTemp(dir);
}

void SkyComponent::SetIrradianceTexturePath(std::string path)
{
	m_irradianceTexturePath = cd::MoveTemp(path);
}

void SkyComponent::SetRadianceTexturePath(std::string path)
{
	m_radianceTexturePath = cd::MoveTemp(path);
}

void SkyComponent::SetPureGrayTexturePath(std::string path)
{
	m_pureGrayTexturePath = cd::MoveTemp(path);
}

}