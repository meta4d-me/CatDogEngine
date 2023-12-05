#include "ResourceBuilder.h"

#include "Base/Template.h"
#include "Log/Log.h"
#include "Path/Path.h"
#include "Time/Clock.h"

#include <cassert>

namespace editor
{

ResourceBuilder::ResourceBuilder()
{
	std::string modifyCachePath = GetModifyCacheFilePath();
	if (engine::Path::FileExists(modifyCachePath.c_str()))
	{
		ReadModifyCacheFile();
	}
}

ResourceBuilder::~ResourceBuilder()
{
	WriteModifyCacheFile();
}

void ResourceBuilder::ReadModifyCacheFile()
{
	std::string modifyCachePath = GetModifyCacheFilePath();
	std::ifstream inFile(modifyCachePath);
	if (!inFile.is_open())
	{
		CD_ERROR("Open file {0} failed!", modifyCachePath);
		return;
	}

	CD_INFO("Reading modify time cache from {0}.", modifyCachePath);

	std::string line;
	while (std::getline(inFile, line))
	{
		size_t pos = line.find("=");
		if (pos != std::string::npos)
		{
			std::string filePath = line.substr(0, pos);

			long long timeStamp = std::stoll(line.substr(pos + 1));
			m_modifyTimeCache[filePath] = timeStamp;
		}
	}

	inFile.close();
}

void ResourceBuilder::WriteModifyCacheFile()
{
	if (!HasNewModifyTimeCache())
	{
		return;
	}

	std::string modifyCachePath = GetModifyCacheFilePath();

	if (!engine::Path::FileExists(modifyCachePath.c_str()))
	{
		CD_INFO("Creating modify cache file at : {0}", modifyCachePath);
		std::filesystem::create_directories(std::filesystem::path(modifyCachePath).parent_path());
	}

	std::ofstream outFile(modifyCachePath, std::ios::trunc);
	if (!outFile.is_open())
	{
		CD_ERROR("Open file {0} failed!", modifyCachePath);
		return;
	}

	CD_INFO("Writing modify time cache to {0}.", modifyCachePath);

	UpdateModifyTimeCache();

	outFile.clear();
	for (auto& [filePath, timeStamp] : m_modifyTimeCache)
	{
		outFile << filePath << "=" << timeStamp << std::endl;
	}

	outFile.close();
}

std::string ResourceBuilder::GetModifyCacheFilePath()
{
	const auto& appDataPath = engine::Path::GetApplicationDataPath();
	if (appDataPath.has_value())
	{
		return (appDataPath.value() / engine::Path::EngineName / "modifyTimeCache.bin").string();
	}

	CD_ERROR("Can not find application data path!");
	return "";
}

ProcessStatus ResourceBuilder::CheckFileStatus(const char* pInputFilePath, const char* pOutputFilePath)
{
	// Use output file path as map key to store time cache.
	// 
	// For normal resources, the input and output files are one-to-one,
	// so the output file path is sufficient to represent the input file.
	// 
	// And for uber shader, a single source file can generate multiple output files,
	// so the input file path is no longer sufficient to represent the modified state of a particular variant.
	
	const char* key = pOutputFilePath;

	if (!engine::Path::FileExists(pInputFilePath))
	{
		CD_ERROR("Input file path {0} does not exists!", pInputFilePath);
		return ProcessStatus::InputNotExist;
	}

	auto crtTimeStamp = engine::Clock::FileTimePointToTimeStamp(std::filesystem::last_write_time(pInputFilePath));

	if (m_modifyTimeCache.find(key) == m_modifyTimeCache.end())
	{
		CD_INFO("New input file {0} detected.", pInputFilePath);
		m_newModifyTimeCache[key] = crtTimeStamp;
		return ProcessStatus::InputAdded;
	}

	if (m_modifyTimeCache[key] != crtTimeStamp)
	{
		CD_INFO("Input file path {0} has been modified.", pInputFilePath);
		m_newModifyTimeCache[key] = crtTimeStamp;
		return ProcessStatus::InputModified;
	}

	if (!engine::Path::FileExists(pOutputFilePath))
	{
		CD_INFO("Output file path {0} dose not exists.", pOutputFilePath);
		return ProcessStatus::OutputNotExist;
	}
	else
	{
		CD_TRACE("Output file path {0} already exists.", pOutputFilePath);
		return ProcessStatus::Stable;
	}

	CD_ERROR("Unknow ProcessStatus of input {0} and output {1}!", pInputFilePath, pOutputFilePath);
	assert(false);
	return ProcessStatus::None;
}


bool ResourceBuilder::AddTask(Process process)
{
	m_buildTasks.push(cd::MoveTemp(process));
	return true;
}

bool ResourceBuilder::AddShaderBuildTask(engine::ShaderType shaderType, const char* pInputFilePath, const char* pOutputFilePath, const char* pShaderFeatures)
{
	if (s_SkipStatus & static_cast<uint8_t>(CheckFileStatus(pInputFilePath, pOutputFilePath)))
	{
		return false;
	}

	// Document : https://bkaradzic.github.io/bgfx/tools.html#shader-compiler-shaderc
	std::string cmftExePath = CDENGINE_TOOL_PATH;
	cmftExePath += "/shaderc";
	Process process(cmftExePath.c_str());

	std::filesystem::path shaderSourceFolderPath(pInputFilePath);
	shaderSourceFolderPath = shaderSourceFolderPath.parent_path();
	shaderSourceFolderPath += "/varying.def.sc";

	std::vector<std::string> commandArguments{ "-f", pInputFilePath, "--varyingdef", shaderSourceFolderPath.string().c_str(),
		"-o", pOutputFilePath, "-O", "3"};
	
	commandArguments.push_back("--platform");
#if CD_PLATFORM_OSX
	commandArguments.push_back("osx");
#elif CD_PLATFORM_IOS
	commandArguments.push_back("ios");
#elif CD_PLATFORM_WINDOWS
	commandArguments.push_back("windows");
#elif CD_PLATFORM_ANDROID || CD_PLATFORM_LINUX
	commandArguments.push_back("android");
#else
	static_assert("CD_PLATFORM macro not defined!");
#endif

	commandArguments.push_back("--type");
	if (engine::ShaderType::Compute == shaderType)
	{
		commandArguments.push_back("c");
	}
	else if (engine::ShaderType::Fragment == shaderType)
	{
		commandArguments.push_back("f");
	}
	else if (engine::ShaderType::Vertex == shaderType)
	{
		commandArguments.push_back("v");
	}

	std::string shaderLanguageDefine;
	commandArguments.push_back("-p");
	switch (engine::Path::GetGraphicsBackend())
	{
	case engine::GraphicsBackend::Direct3D11:
	case engine::GraphicsBackend::Direct3D12:
		commandArguments.push_back("s_5_0");
		shaderLanguageDefine = "BGFX_SHADER_LANGUAGE_HLSL";
		break;
	case engine::GraphicsBackend::OpenGL:
		commandArguments.push_back("440");
		shaderLanguageDefine = "BGFX_SHADER_LANGUAGE_GLSL";
		break;
	case engine::GraphicsBackend::OpenGLES:
		commandArguments.push_back("320_es");
		shaderLanguageDefine = "BGFX_SHADER_LANGUAGE_GLSL";
		break;
	case engine::GraphicsBackend::Metal:
		commandArguments.push_back("metal");
		shaderLanguageDefine = "BGFX_SHADER_LANGUAGE_METAL";
		break;
	case engine::GraphicsBackend::Vulkan:
		commandArguments.push_back("spirv15-12");
		shaderLanguageDefine = "BGFX_SHADER_LANGUAGE_SPIRV";
		break;
	default:
		assert("Unknown shader compile profile.");
	}

	if (std::strlen(pShaderFeatures) != 0 && std::strlen(pShaderFeatures))
	{
		commandArguments.push_back("--define");
		commandArguments.push_back(shaderLanguageDefine + ";" + pShaderFeatures);
	}

	process.SetCommandArguments(cd::MoveTemp(commandArguments));
	AddTask(cd::MoveTemp(process));

	return true;
}

bool ResourceBuilder::AddIrradianceCubeMapBuildTask(const char* pInputFilePath, const char* pOutputFilePath)
{
	if (s_SkipStatus & static_cast<uint8_t>(CheckFileStatus(pInputFilePath, pOutputFilePath)))
	{
		return false;
	}

	std::string cmftExePath = (std::filesystem::path(CDENGINE_TOOL_PATH) / "cmft").generic_string();
	Process process(cmftExePath.c_str());
	std::string pathWithoutExtension = std::filesystem::path(pOutputFilePath).replace_extension().generic_string();

	std::vector<std::string> irradianceCommandArguments{"--input", pInputFilePath,
		"--filter", "irradiance",
		"--dstFaceSize", "256",
		"--outputNum", "1", "--output0", cd::MoveTemp(pathWithoutExtension), "--output0params", "dds,rgba16f,cubemap"};

	process.SetCommandArguments(cd::MoveTemp(irradianceCommandArguments));
	AddTask(cd::MoveTemp(process));

	return true;
}

bool ResourceBuilder::AddRadianceCubeMapBuildTask(const char* pInputFilePath, const char* pOutputFilePath)
{
	if (s_SkipStatus & static_cast<uint8_t>(CheckFileStatus(pInputFilePath, pOutputFilePath)))
	{
		return false;
	}

	std::string cmftExePath = (std::filesystem::path(CDENGINE_TOOL_PATH) / "cmft").generic_string();
	Process process(cmftExePath.c_str());
	std::string pathWithoutExtension = std::filesystem::path(pOutputFilePath).replace_extension().generic_string();

	std::vector<std::string> radianceCommandArguments{"--input", pInputFilePath,
		"--filter", "radiance", "--lightingModel", "phongbrdf", "--excludeBase", "true", "--mipCount", "7",
		"--dstFaceSize", "256",
		"--outputNum", "1", "--output0", cd::MoveTemp(pathWithoutExtension), "--output0params", "dds,rgba16f,cubemap"};

	process.SetCommandArguments(cd::MoveTemp(radianceCommandArguments));
	AddTask(cd::MoveTemp(process));

	return true;
}

bool ResourceBuilder::AddTextureBuildTask(cd::MaterialTextureType textureType, const char* pInputFilePath, const char* pOutputFilePath)
{
	if (s_SkipStatus & static_cast<uint8_t>(CheckFileStatus(pInputFilePath, pOutputFilePath)))
	{
		return false;
	}

	// Document : https://bkaradzic.github.io/bgfx/tools.html#texture-compiler-texturec
	std::string texturecExePath = CDENGINE_TOOL_PATH;
	texturecExePath += "/texturec";
	Process process(texturecExePath.c_str());

	std::vector<std::string> commandArguments{ "-f", pInputFilePath, "-o", pOutputFilePath, "-t", "BC3", "--mips", "-q", "highest", "--max", "1024"};
	if (cd::MaterialTextureType::Normal == textureType)
	{
		commandArguments.push_back("--normalmap");
	}
	else if (cd::MaterialTextureType::BaseColor != textureType)
	{
		commandArguments.push_back("--linear");
	}
	process.SetCommandArguments(cd::MoveTemp(commandArguments));
	AddTask(cd::MoveTemp(process));

	return true;
}

void ResourceBuilder::Update(bool doPrintErrorLog, bool doPrintLog)
{
	if (m_buildTasks.empty())
	{
		return;
	}

	// It may wait until process exited which depends on process's setting.
	while (!m_buildTasks.empty())
	{
		Process& process = m_buildTasks.front();
		process.SetWaitUntilFinished(m_buildTasks.size() == 1);
		process.SetPrintChildProcessLog(doPrintLog);
		process.SetPrintChildProcessErrorLog(doPrintErrorLog);
		process.Run();
		m_buildTasks.pop();
	}

	if (m_buildTasks.empty())
	{
		WriteModifyCacheFile();
	}
}

bool ResourceBuilder::HasNewModifyTimeCache() const
{
	return !m_newModifyTimeCache.empty();
}

void ResourceBuilder::UpdateModifyTimeCache()
{
	for (auto& [filePath, fileTime] : m_newModifyTimeCache)
	{
		m_modifyTimeCache[cd::MoveTemp(filePath)] = cd::MoveTemp(fileTime);
	}

	m_newModifyTimeCache.clear();
}

void ResourceBuilder::ClearModifyTimeCache()
{
	m_modifyTimeCache.clear();
	m_newModifyTimeCache.clear();
}

void ResourceBuilder::DeleteModifyTimeCache()
{
	if (!std::filesystem::remove(GetModifyCacheFilePath()))
	{
		CD_WARN("Delete file {0} failed!", GetModifyCacheFilePath());
	}
}

}