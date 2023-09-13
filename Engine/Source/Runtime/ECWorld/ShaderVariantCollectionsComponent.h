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
	std::map<std::string, ShaderProgramPack>& GetShaderPrograms() { return m_shaderPrograms; }
	const std::map<std::string, ShaderProgramPack>& GetShaderPrograms() const { return m_shaderPrograms; }

private:
	inline bool IsProgramValid(std::string programName) const;

	// Key : Program Name, Value : Shader Path + Variant.
	std::map<std::string, ShaderProgramPack> m_shaderPrograms;
};

}