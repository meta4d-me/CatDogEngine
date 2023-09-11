#pragma once

#include "Base/Template.h"
#include "Core/StringCrc.h"
#include "Rendering/ShaderFeature.h"

#include <map>
#include <optional>

namespace engine
{

class ShaderVariantCollectionsComponent final
{
public:
	static constexpr StringCrc GetClassName()
	{
		constexpr StringCrc className("ShaderVariantCollectionsComponent");
		return className;
	}

	ShaderVariantCollectionsComponent() = default;
	ShaderVariantCollectionsComponent(const ShaderVariantCollectionsComponent&) = default;
	ShaderVariantCollectionsComponent& operator=(const ShaderVariantCollectionsComponent&) = default;
	ShaderVariantCollectionsComponent(ShaderVariantCollectionsComponent&&) = default;
	ShaderVariantCollectionsComponent& operator=(ShaderVariantCollectionsComponent&&) = default;
	~ShaderVariantCollectionsComponent() = default;

	void AddShader(std::string path, ShaderFeatureSet set = {});
	void ActiveShaderFeature(std::string path, ShaderFeature feature);
	void DeactiveShaderFeature(std::string path, ShaderFeature feature);

	void SetShaderFeatureSet(std::string path, ShaderFeatureSet set);
	ShaderFeatureSet& GetShaderFeatureSet(std::string path);
	const ShaderFeatureSet& GetShaderFeatureSet(std::string path) const;

	void SetShaderInformations(std::map<std::string, ShaderFeatureSet> info);
	std::map<std::string, ShaderFeatureSet>& GetShaderInformations() { return m_shaderInformations; }
	const std::map<std::string, ShaderFeatureSet>& GetShaderInformations() const { return m_shaderInformations; }

private:
	bool inline IsValid(std::string path) const;

	// Key : Shader source file path, Value : Shader feature set.
	// Non-Uber shader has no features, in this case the value will be std::nullopt.
	std::map<std::string, ShaderFeatureSet> m_shaderInformations;
};

}