#pragma once

namespace engine
{

class MaterialInstance
{
public:
	MaterialInstance() = default;
	MaterialInstance(const MaterialInstance&) = default;
	MaterialInstance& operator=(const MaterialInstance&) = default;
	MaterialInstance(MaterialInstance&&) = default;
	MaterialInstance& operator=(MaterialInstance&&) = default;
	~MaterialInstance() = default;
};

}