#include "SkyComponent.h"

namespace engine
{

void SkyComponent::SetSkyType(SkyType crtType)
{
	if (m_type == crtType)
	{
		return;
	}

	m_type = crtType;
	static std::string preRadPath = m_radianceTexturePath;
	if (SkyType::None == m_type)
	{
		preRadPath = cd::MoveTemp(m_radianceTexturePath);
		m_radianceTexturePath = SkyComponent::PureGrayTexturePath;
	}
	else if(preRadPath != m_radianceTexturePath)
	{
		m_radianceTexturePath = preRadPath;
	} 
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