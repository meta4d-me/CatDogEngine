#pragma once

#include "Core/StringCrc.h"

#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_set>
#include <vector>

namespace engine
{

enum class Uber : uint32_t
{
	DEFAULT = 0,

	// PBR parameters
	ALBEDOMAP,
	NORMALMAP,
	ORMMAP,
	EMISSIVEMAP,

	// Techniques
	IBL,
	ATM,
	AREALLIGHT,

	COUNT,
};

enum class LoadingStatus : uint8_t
{
	MISSING_RESOURCES = 0,
	LOADING_SHADERS,
	LOADING_TEXTURES,
	LOADING_ERROR,

	COUNT,
};

class ShaderSchema
{
public:
	static constexpr uint16_t InvalidProgramHandle = UINT16_MAX;
	static constexpr StringCrc DefaultUberShaderCrc = StringCrc("");
	using ShaderBlob = std::vector<std::byte>;

public:
	ShaderSchema() = default;
	explicit ShaderSchema(std::string vsPath, std::string fsPath);
	ShaderSchema(const ShaderSchema&) = delete;
	ShaderSchema& operator=(const ShaderSchema&) = delete;
	ShaderSchema(ShaderSchema&&) = default;
	ShaderSchema& operator=(ShaderSchema&&) = default;
	~ShaderSchema() = default;

	const char* GetVertexShaderPath() const { return m_vertexShaderPath.c_str(); }
	const char* GetFragmentShaderPath() const { return m_fragmentShaderPath.c_str(); }

	// This option will combien with every exists combination.
	void RegisterUberOption(Uber uberOption);

	bool IsUberOptionValid(StringCrc uberOption) const;
	StringCrc GetOptionsCrc(const std::unordered_set<Uber>& options) const;

	void SetCompiledProgram(StringCrc uberOption, uint16_t programHandle);
	uint16_t GetCompiledProgram(StringCrc uberOption) const;

	const std::vector<Uber>& GetUberOptions() const { return m_uberOptions; }
	const std::vector<std::string>& GetUberCombines() const { return m_uberCombines; }
	const std::map<uint32_t, uint16_t>& GetUberPrograms() const { return m_compiledProgramHandles; }

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

	std::unique_ptr<ShaderBlob> m_pVSBlob;
	std::map<uint32_t, std::unique_ptr<ShaderBlob>> m_uberOptionToFSBlobs;
};

}