#pragma once

namespace engine
{

enum class ResourceStatus
{
	Loading, // Wait to assign asset pointer
	Loaded, // Get asset pointer ready
	Building, // Wait to build render data
	Built, // Built render data in CPU
	Ready, // Already sent to GPU
	Optimized, // Release CPU buffer to optimize memory usage
	Garbage, // Wait to destroy and cleanup
	Destroyed,
};

enum class ResourceType
{
	Mesh,
	Texture,
};

class IResource
{
public:
	IResource() = default;
	IResource(const IResource&) = delete;
	IResource& operator=(const IResource&) = delete;
	IResource(IResource&&) = delete;
	IResource& operator=(IResource&&) = delete;
	virtual ~IResource() = default;

	virtual void Update() = 0;
	ResourceStatus GetStatus() const { return m_status; }
	void SetStatus(ResourceStatus status) { m_status = status; }

private:
	ResourceStatus m_status = ResourceStatus::Loading;
};

}