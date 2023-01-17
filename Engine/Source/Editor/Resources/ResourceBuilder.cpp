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
}

void ResourceBuilder::AddCubeMapBuildTask(const char* pInputFilePath, const char* pOutputFileName)
{
	// Document : In command line, ./cmft -h
	std::string cmftExePath = CDENGINE_TOOL_PATH;
	cmftExePath += "/cmft";
	Process process(cmftExePath.c_str());

	std::vector<std::string> commandArguments{ "--input", pInputFilePath,
		"--outputNum", "1", "--output0", pOutputFileName,
		"--excludeBase", "true", "--mipCount", "7", "--glossScale", "10", "--glossBias", "2", "--edgeFixup", "warp" };
	process.SetCommandArguments(cd::MoveTemp(commandArguments));
	process.SetWaitUntilFinished(true);
	AddTask(cd::MoveTemp(process));
}

void ResourceBuilder::AddTextureBuildTask(const char* pInputFilePath, const char* pOutputFilePath, cd::MaterialTextureType textureType)
{
	// Document : https://bkaradzic.github.io/bgfx/tools.html#texture-compiler-texturec
	std::string texturecExePath = CDENGINE_TOOL_PATH;
	texturecExePath += "/texturec";
	Process process(texturecExePath.c_str());

	std::vector<std::string> commandArguments{ "-f", pInputFilePath, "-o", pOutputFilePath, "-t", "BC3", "--mips", "-q", "fastest"};
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
	process.SetWaitUntilFinished(true);
	AddTask(cd::MoveTemp(process));
}

void ResourceBuilder::Update()
{
	// It may wait until process exited which depends on process's setting.
	while (!m_buildTasks.empty())
	{
		Process& process = m_buildTasks.front();
		process.Run();
		m_buildTasks.pop();
	}
}

}