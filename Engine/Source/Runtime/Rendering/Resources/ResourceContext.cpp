#include "ResourceContext.h"

#include "Base/NameOf.h"
#include "MeshResource.h"

namespace engine
{

ResourceContext::~ResourceContext()
{
	// TODO
}

void ResourceContext::Update()
{
	for (auto& [_, resource] : m_resources)
	{
		resource->Update();
	}
}

StringCrc ResourceContext::GetResourceCrc(ResourceType resourceType, StringCrc nameCrc)
{
	StringCrc resourceCrc{ nameof::nameof_enum(resourceType) };
	resourceCrc.Set(resourceCrc.Value() + nameCrc.Value());
	return resourceCrc;
}

MeshResource* ResourceContext::AddMeshResource(StringCrc nameCrc)
{
	StringCrc resourceCrc = GetResourceCrc(ResourceType::Mesh, nameCrc);
	auto itResource = m_resources.find(resourceCrc);
	if (itResource != m_resources.end())
	{
		// It is confident to convert as we query Mesh type crc explicitly.
		return static_cast<MeshResource*>(itResource->second.get());
	}
	
	auto meshResource = std::make_unique<MeshResource>();
	auto* pMeshResource = meshResource.get();
	m_resources[resourceCrc] = cd::MoveTemp(meshResource);
	return pMeshResource;
}

}