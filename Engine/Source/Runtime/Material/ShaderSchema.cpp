#include "ShaderSchema.h"

#include "Base/Template.h"

#include <cassert>

namespace engine
{

ShaderSchema::ShaderSchema(std::string vsPath, std::string fsPath)
{
	m_vertexShaderPath = cd::MoveTemp(vsPath);
	m_fragmentShaderPath = cd::MoveTemp(fsPath);
}

void ShaderSchema::RegisterUberOption(std::string uberOption)
{
	m_compiledProgramHandles[StringCrc(uberOption).Value()] = InvalidProgramHandle;
	m_uberOptions.emplace_back(cd::MoveTemp(uberOption));
}

bool ShaderSchema::IsUberOptionValid(StringCrc uberOption) const
{
	return m_compiledProgramHandles.contains(uberOption.Value());
}

void ShaderSchema::SetCompiledProgram(StringCrc uberOption, uint16_t programHandle)
{
	assert(IsUberOptionValid(uberOption));
	m_compiledProgramHandles[uberOption.Value()] = programHandle;
}

uint16_t ShaderSchema::GetCompiledProgram(StringCrc uberOption) const
{
	auto itProgram = m_compiledProgramHandles.find(uberOption.Value());
	assert(itProgram != m_compiledProgramHandles.end());
	uint16_t programHandle = itProgram->second;

	// Registered but not compiled.
	assert(programHandle != InvalidProgramHandle);

	return programHandle;
}

const std::vector<std::string>& ShaderSchema::GetUberOptions() const
{
	return m_uberOptions;
}

const std::map<uint32_t, uint16_t>& ShaderSchema::GetUberPrograms() const
{
	return m_compiledProgramHandles;
}

void ShaderSchema::AddUberOptionVSBlob(ShaderBlob shaderBlob)
{
	m_vsBlob = cd::MoveTemp(shaderBlob);
}

void ShaderSchema::AddUberOptionFSBlob(StringCrc uberOption, ShaderBlob shaderBlob)
{
	m_uberOptionToFSBlobs[uberOption.Value()] = cd::MoveTemp(shaderBlob);
}

const ShaderSchema::ShaderBlob& ShaderSchema::GetVSBlob() const
{
	return m_vsBlob;
}

const ShaderSchema::ShaderBlob& ShaderSchema::GetFSBlob(StringCrc uberOption) const
{
	auto itBlob = m_uberOptionToFSBlobs.find(uberOption.Value());
	assert(itBlob != m_uberOptionToFSBlobs.end());
	return itBlob->second;
}

}