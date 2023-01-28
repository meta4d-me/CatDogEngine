#pragma once

#include "Core/StringCrc.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace engine
{

class ShaderSchema
{
public:
	static constexpr uint16_t InvalidProgramHandle = UINT16_MAX;
	static constexpr char DefaultUberOptionName[] = "DEFAULT";
	static constexpr StringCrc DefaultUberOption = StringCrc(DefaultUberOptionName);
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

	void RegisterUberOption(std::string uberOption);
	bool IsUberOptionValid(StringCrc uberOption) const;
	void SetCompiledProgram(StringCrc uberOption, uint16_t programHandle);
	uint16_t GetCompiledProgram(StringCrc uberOption) const;
	const std::vector<std::string>& GetUberOptions() const { return m_uberOptions; }
	const std::map<uint32_t, uint16_t>& GetUberPrograms() const { return m_compiledProgramHandles; }

	// TODO : More generic.
	void AddUberOptionVSBlob(ShaderBlob shaderBlob);
	void AddUberOptionFSBlob(StringCrc uberOption, ShaderBlob shaderBlob);
	const ShaderBlob& GetVSBlob() const { return *m_pVSBlob.get(); }
	const ShaderBlob& GetFSBlob(StringCrc uberOption) const;

private:
	std::string m_vertexShaderPath;
	std::string m_fragmentShaderPath;
	std::vector<std::string> m_uberOptions;

	std::unique_ptr<ShaderBlob> m_pVSBlob;
	std::map<uint32_t, std::unique_ptr<ShaderBlob>> m_uberOptionToFSBlobs;
	std::map<uint32_t, uint16_t> m_compiledProgramHandles;
};

}