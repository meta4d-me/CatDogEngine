#include "ResourceBuilder.h"

#include "Base/Template.h"

namespace editor
{

void ResourceBuilder::AddTask(Process process)
{
	m_buildTasks.push(cd::MoveTemp(process));
}

void ResourceBuilder::AddShaderBuildTask(const char* pInputFilePath, const char* pOutputFilePath)
{
	//Process process("shaderc");
	//AddTask(cd::MoveTemp(process));
}

void ResourceBuilder::AddTextureBuildTask(const char* pInputFilePath, const char* pOutputFilePath, cd::MaterialTextureType textureType)
{
	// Document : https://bkaradzic.github.io/bgfx/tools.html#texture-compiler-texturec
	Process process("texturec");

	std::vector<std::string> commandArguments{ "-f", pInputFilePath, "-o", pOutputFilePath, "-t BC3", "--mips", "-q highest" };
	if (cd::MaterialTextureType::Normal == textureType)
	{
		commandArguments.push_back("--normalmap");
	}
	else if (cd::MaterialTextureType::Metalness == textureType ||
		cd::MaterialTextureType::Roughness == textureType)
	{
		commandArguments.push_back("--linear");
	}
	process.SetCommandArguments(cd::MoveTemp(commandArguments));

	AddTask(cd::MoveTemp(process));
}

void ResourceBuilder::Update()
{
	if (m_buildTasks.empty())
	{
		return;
	}

	// It may wait until process exited which depends on process's setting.
	Process& process = m_buildTasks.front();
	process.Run();
	m_buildTasks.pop();
}

}