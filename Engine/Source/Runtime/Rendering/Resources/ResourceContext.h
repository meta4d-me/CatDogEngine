#pragma once

#include "Core/StringCrc.h"

#include <map>
#include <memory>

namespace engine
{

enum class ResourceType;
class IResource;
class MeshResource;

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

private:
	std::map<StringCrc, std::unique_ptr<IResource>> m_resources;
};

}