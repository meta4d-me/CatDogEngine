#pragma once

#include "Base/Template.h"
#include "Core/StringCrc.h"
#include "Rendering/ShaderProgramPack.h"

#include <map>
#include <memory>
#include <string>

namespace engine
{

class ShaderVariantCollectionsComponent final
{
public:
		using ShaderBlob = std::vector<std::byte>;

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

	void RegisterPragram(std::string programName, ShaderProgramPack pack);

	void ActivateShaderFeature(std::string programName, ShaderFeature feature);
	void DeactiveShaderFeature(std::string programName, ShaderFeature feature);

	void SetShaderPrograms(std::map<std::string, ShaderProgramPack> program);
	std::map<std::string, ShaderProgramPack>& GetShaderInformations() { return m_shaderPrograms; }
	const std::map<std::string, ShaderProgramPack>& GetShaderInformations() const { return m_shaderPrograms; }

private:
	inline bool IsValid(std::string path) const;

	// Key : Shader source file path, Value : Shader feature set.
	// Non-Uber shader has no features, in this case the value will be an empty std::set.
	// std::map<std::string, ShaderFeatureSet> m_shaderInformations;

	// Key : Program Name, Value : VS Path + FS Path + variant.
	std::map<std::string, ShaderProgramPack> m_shaderPrograms;

	// Key : Compiled Binary Shader File Name, Value : File Data.
	// std::map<std::string, std::unique_ptr<ShaderBlob>> m_shaderData;
};

}