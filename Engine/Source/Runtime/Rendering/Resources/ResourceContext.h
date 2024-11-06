#pragma once

#include "Core/StringCrc.h"

#include <map>
#include <memory>

namespace engine
{

enum class ResourceType;
class IResource;
class MeshResource;
class ShaderResource;
class SkeletonResource;
class TextureResource;

class ResourceContext
{
public:
	ResourceContext() = default;
	ResourceContext(const ResourceContext&) = delete;
	ResourceContext& operator=(const ResourceContext&) = delete;
	ResourceContext(ResourceContext&&) = delete;
	ResourceContext& operator=(ResourceContext&&) = delete;
	~ResourceContext();

	void Update();

	StringCrc GetResourceCrc(ResourceType resourceType, StringCrc nameCrc);

	MeshResource* AddMeshResource(StringCrc nameCrc);
	ShaderResource* AddShaderResource(StringCrc nameCrc);
	SkeletonResource* AddSkeletonResource(StringCrc nameCrc);
	TextureResource* AddTextureResource(StringCrc nameCrc);
	MeshResource* GetMeshResource(StringCrc nameCrc);
	ShaderResource* GetShaderResource(StringCrc nameCrc);
	TextureResource* GetTextureResource(StringCrc nameCrc);

private:
	template<ResourceType RT>
	IResource* AddResourceImpl(StringCrc nameCrc);

	template<ResourceType RT>
	IResource* GetResourceImpl(StringCrc nameCrc);

private:
	std::map<StringCrc, std::unique_ptr<IResource>> m_resources;
};

}