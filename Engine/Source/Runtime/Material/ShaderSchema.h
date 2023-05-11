#pragma once

#include "Base/Template.h"
#include "Core/StringCrc.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace engine
{

enum class Uber : uint32_t
{
	DEFAULT = 0,

	// PBR parameters
	ALBEDO,
	NORMAL_MAP,
	OCCLUSION,
	ROUGHNESS,
	METALLIC,

	// Techniques
	IBL,
	AREAL_LIGHT,

	COUNT,
};

DEFINE_ENUM_WITH_NAMES(LoadingStatus, MISSING_RESOURCES, LOADING_SHADERS, LOADING_TEXTURES, LOADING_ERROR);

class ShaderSchema
{
public:
	static constexpr uint16_t InvalidProgramHandle = UINT16_MAX;
	static constexpr StringCrc DefaultUberOption = StringCrc("");
	using ShaderBlob = std::vector<std::byte>;

public:
	ShaderSchema() = default;
	explicit ShaderSchema(std::string vsPath, std::string fsPath);
	ShaderSchema(const ShaderSchema&) = default;
	ShaderSchema& operator=(const ShaderSchema&) = default;
	ShaderSchema(ShaderSchema&&) = default;
	ShaderSchema& operator=(ShaderSchema&&) = default;
	~ShaderSchema() = default;

	const char* GetVertexShaderPath() const { return m_vertexShaderPath.c_str(); }
	const char* GetFragmentShaderPath() const { return m_fragmentShaderPath.c_str(); }

	// This option will combien with every exists combination.
	void RegisterUberOption(Uber uberOption);
	// Add a single option wihch will not combine with any other option.
	void AddSingleUberOption(LoadingStatus status, std::string path);

	bool IsUberOptionValid(StringCrc uberOption) const;
	StringCrc GetProgramCrc(const std::vector<Uber>& options) const;
	StringCrc GetProgramCrc(const LoadingStatus& status) const;

	void SetCompiledProgram(StringCrc uberOption, uint16_t programHandle);
	uint16_t GetCompiledProgram(StringCrc uberOption) const;

	const std::vector<Uber>& GetUberOptions() const { return m_uberOptions; }
	const std::vector<std::string>& GetUberCombines() const { return m_uberCombines; }
	const std::map<uint32_t, uint16_t>& GetUberPrograms() const { return m_compiledProgramHandles; }
	const std::map<LoadingStatus, std::string>& GetLoadingStatusPath() const { return m_loadingStatusFSPath; }

	// TODO : More generic.
	void AddUberOptionVSBlob(ShaderBlob shaderBlob);
	void AddUberOptionFSBlob(StringCrc uberOption, ShaderBlob shaderBlob);
	const ShaderBlob& GetVSBlob() const { return *m_pVSBlob.get(); }
	const ShaderBlob& GetFSBlob(StringCrc uberOption) const;

private:
	std::string m_vertexShaderPath;
	std::string m_fragmentShaderPath;

	// Registration order of options. 
	std::vector<Uber> m_uberOptions;
	// Parameters to compile shaders.
	std::vector<std::string> m_uberCombines;
	// Key: StringCrc(option combine), Value: shader handle.
	std::map<uint32_t, uint16_t> m_compiledProgramHandles;
	// Key: LoadingStatus, Value: fragment shader path.
	std::map<LoadingStatus, std::string> m_loadingStatusFSPath;

	std::unique_ptr<ShaderBlob> m_pVSBlob;
	std::map<uint32_t, std::unique_ptr<ShaderBlob>> m_uberOptionToFSBlobs;
};

}