#include "NameComponent.h"

#include "Base/Template.h"

namespace engine
{

void NameComponent::SetName(std::string name)
{
	m_name = cd::MoveTemp(name);
	m_nameCrc = StringCrc(m_name);
}

}