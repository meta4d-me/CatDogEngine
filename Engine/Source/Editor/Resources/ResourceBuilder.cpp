#include "ResourceBuilder.h"

#include "Base/Template.h"

#include <filesystem>

namespace editor
{

void ResourceBuilder::AddTask(Process process)
{
	m_buildTasks.push(cd::MoveTemp(process));
}

void ResourceBuilder::AddShaderBuildTask(const char* pInputFilePath, const char* pOutputFilePath, ShaderType shaderType)
{
	// Document : https://bkaradzic.github.io/bgfx/tools.html#shader-compiler-shaderc
	std::string cmftExePath = CDENGINE_TOOL_PATH;
	cmftExePath += "/shaderc";
	Process process(cmftExePath.c_str());

	std::filesystem::path shaderSourceFilePath = pInputFilePath;
	std::filesystem::path shaderSourceFolderPath = shaderSourceFilePath.parent_path();
	shaderSourceFolderPath += "/varying.def.sc";

	std::vector<std::string> commandArguments{ "-f", pInputFilePath, "--varyingdef", shaderSourceFolderPath.string().c_str(),
		"-o", pOutputFilePath, "--platform", "windows", "-O", "3"};
	
	if (ShaderType::Compute == shaderType)
	{
		commandArguments.push_back("--type");
		commandArguments.push_back("c");
		commandArguments.push_back("-p");
		commandArguments.push_back("cs_5_0");
	}
	else if (ShaderType::Fragment == shaderType)
	{
		commandArguments.push_back("--type");
		commandArguments.push_back("f");
		commandArguments.push_back("-p");
		commandArguments.push_back("ps_5_0");
	}
	else if (ShaderType::Vertex == shaderType)
	{
		commandArguments.push_back("--type");
		commandArguments.push_back("v");
		commandArguments.push_back("-p");
		commandArguments.push_back("vs_5_0");
	}

	process.SetCommandArguments(cd::MoveTemp(commandArguments));

	process.SetWaitUntilFinished(true);
	AddTask(cd::MoveTemp(process));
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
	else if (cd::MaterialTextureType::BaseColor != textureType)
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