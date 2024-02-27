#include "ResourceContext.h"

#include "Base/NameOf.h"
#include "MeshResource.h"
#include "TextureResource.h"

namespace engine
{

ResourceContext::~ResourceContext()
{
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
	return static_cast<MeshResource*>(AddResourceImpl<ResourceType::Mesh>(nameCrc));
}

TextureResource* ResourceContext::AddTextureResource(StringCrc nameCrc)
{
	return static_cast<TextureResource*>(AddResourceImpl<ResourceType::Texture>(nameCrc));
}

MeshResource* ResourceContext::GetMeshResource(StringCrc nameCrc)
{
	return static_cast<MeshResource*>(GetResourceImpl<ResourceType::Mesh>(nameCrc));
}

TextureResource* ResourceContext::GetTextureResource(StringCrc nameCrc)
{
	return static_cast<TextureResource*>(GetResourceImpl<ResourceType::Texture>(nameCrc));
}

template<ResourceType RT>
IResource* ResourceContext::AddResourceImpl(StringCrc nameCrc)
{
	StringCrc resourceCrc = GetResourceCrc(RT, nameCrc);
	auto itResource = m_resources.find(resourceCrc);
	if (itResource != m_resources.end())
	{
		return itResource->second.get();
	}

	if constexpr (ResourceType::Mesh == RT)
	{
		m_resources[resourceCrc] = std::make_unique<MeshResource>();
	}
	else if constexpr (ResourceType::Texture == RT)
	{
		m_resources[resourceCrc] = std::make_unique<TextureResource>();
	}

	auto* pResource = m_resources[resourceCrc].get();
	pResource->SetName(nameCrc);
	return pResource;
}

template<ResourceType RT>
IResource* ResourceContext::GetResourceImpl(StringCrc nameCrc)
{
	StringCrc resourceCrc = GetResourceCrc(RT, nameCrc);
	auto itResource = m_resources.find(resourceCrc);
	return itResource != m_resources.end() ? itResource->second.get() : nullptr;
}

}