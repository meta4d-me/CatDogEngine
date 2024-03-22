#include "ShaderResource.h"

#include "Log/Log.h"
#include "Path/Path.h"
#include "Resources/ResourceLoader.h"

#include <bgfx/bgfx.h>

#include <cassert>

namespace engine
{

ShaderResource::~ShaderResource()
{
	SetStatus(ResourceStatus::Garbage);
	Update();
}

void ShaderResource::Update()
{
	switch (GetStatus())
	{
		case ResourceStatus::Loading:
		{
			// TODO : Should integrate the "compiling shader sc file to bin file" process in ShaderResource::Update.
			// For now, we just read the shader bin file.

			if (LoadShader())
			{
				SetStatus(ResourceStatus::Loaded);
			}
			break;
		}
		case ResourceStatus::Loaded:
		{
			if (!m_shaders[0].binBlob.empty() && !(ShaderProgramType::Standard == m_type && m_shaders[1].binBlob.empty()))
			{
				// It seems no Rendering data for shader? Skip Building status.
				SetStatus(ResourceStatus::Built);
			}
			break;
		}
		case ResourceStatus::Building:
		{
			SetStatus(ResourceStatus::Built);
			break;
		}
		case ResourceStatus::Built:
		{
			// Build GPU handles
			if (BuildShaderHandle() && BuildProgramHandle())
			{
				m_recycleCount = 0U;
				SetStatus(ResourceStatus::Ready);
			}
			break;
		}
		case ResourceStatus::Ready:
		{
			// Delete CPU data delayed
			constexpr uint32_t recycleDelayFrames = 30U;
			if (m_recycleCount++ >= recycleDelayFrames)
			{
				FreeShaderData(0);
				FreeShaderData(1);

				m_recycleCount = 0U;
				SetStatus(ResourceStatus::Optimized);
			}
			break;
		}
		case ResourceStatus::Garbage:
		{
			// Distory GPU handles
			DistoryShaderHandle(0);
			DistoryShaderHandle(1);
			DistoryProgramHandle();
			SetStatus(ResourceStatus::Destroyed);
			break;
		}
		default:
			break;
	}
}
void ShaderResource::Reset()
{
	ClearShaderData(0);
	ClearShaderData(1);
	DistoryShaderHandle(0);
	DistoryShaderHandle(1);
	DistoryProgramHandle();
	SetStatus(ResourceStatus::Loading);
}

void ShaderResource::SetShaders(const std::string& vsName, const std::string& fsName, const std::string& combine)
{
	assert(ShaderProgramType::Standard == m_type);

	m_shaders[0].type = ShaderType::Vertex;
	m_shaders[0].name = vsName;
	m_shaders[0].scPath = engine::Path::GetBuiltinShaderInputPath(vsName.c_str());
	// TODO : Uber Vertex Shader
	m_shaders[0].binPath = engine::Path::GetShaderOutputPath(vsName.c_str());
	
	m_shaders[1].type = ShaderType::Fragment;
	m_shaders[1].name = fsName;
	m_shaders[1].scPath = engine::Path::GetBuiltinShaderInputPath(fsName.c_str());
	m_shaders[1].binPath = engine::Path::GetShaderOutputPath(fsName.c_str(), combine);

	m_featuresCombine = combine;
}

void ShaderResource::SetShader(ShaderType type, const std::string& name, const std::string& combine)
{
	assert(ShaderProgramType::Standard != m_type);

	m_shaders[0].type = type;
	m_shaders[0].name = name;
	m_shaders[0].scPath = engine::Path::GetBuiltinShaderInputPath(name.c_str());
	m_shaders[0].binPath = engine::Path::GetShaderOutputPath(name.c_str(), combine);

	m_featuresCombine = combine;
}

void ShaderResource::SetShaderInfo(ShaderInfo info, size_t index)
{
	CheckIndex(index);
	m_shaders[index] = cd::MoveTemp(info);
}

ShaderResource::ShaderInfo& ShaderResource::GetShaderInfo(size_t index)
{
	CheckIndex(index);
	return m_shaders[index];
}

const ShaderResource::ShaderInfo& ShaderResource::GetShaderInfo(size_t index) const
{
	CheckIndex(index);
	return m_shaders[index];
}

bool ShaderResource::LoadShader()
{
	auto& shader = m_shaders[0];
	if (!Path::FileExists(shader.binPath.c_str()))
	{
		return false;
	}
	shader.binBlob = engine::ResourceLoader::LoadFile(shader.binPath.c_str());

	if (ShaderProgramType::Standard == m_type)
	{
		auto& fragmentShader = m_shaders[1];
		if (!Path::FileExists(fragmentShader.binPath.c_str()))
		{
			ClearShaderData(0);
			return false;
		}
		fragmentShader.binBlob = engine::ResourceLoader::LoadFile(fragmentShader.binPath.c_str());
	}

	return true;
}

bool ShaderResource::BuildShaderHandle()
{
	if (m_shaders[0].binBlob.empty())
	{
		return false;
	}

	assert(!bgfx::isValid(bgfx::ShaderHandle{ m_shaders[0].handle }));
	bgfx::ShaderHandle handle = bgfx::createShader(bgfx::makeRef(m_shaders[0].binBlob.data(), static_cast<uint32_t>(m_shaders[0].binBlob.size())));
	if (!bgfx::isValid(handle))
	{
		ClearShaderData(0);
		return false;
	}
	m_shaders[0].handle = handle.idx;

	if (ShaderProgramType::Standard == m_type)
	{
		if (m_shaders[1].binBlob.empty())
		{
			ClearShaderData(0);
			DistoryShaderHandle(0);
			return false;
		}

		assert(!bgfx::isValid(bgfx::ShaderHandle{ m_shaders[1].handle }));
		bgfx::ShaderHandle fragmentShaderHandle = bgfx::createShader(bgfx::makeRef(m_shaders[1].binBlob.data(), static_cast<uint32_t>(m_shaders[1].binBlob.size())));
		if (!bgfx::isValid(fragmentShaderHandle))
		{
			ClearShaderData(0);
			ClearShaderData(1);
			DistoryShaderHandle(0);
			return false;
		}
		m_shaders[1].handle = fragmentShaderHandle.idx;
	}

	return true;
}

bool ShaderResource::BuildProgramHandle()
{
	assert(!bgfx::isValid(bgfx::ProgramHandle{ m_programHandle }));

	if (ShaderProgramType::Standard == m_type)
	{
		m_programHandle = bgfx::createProgram(bgfx::ShaderHandle{ m_shaders[0].handle }, bgfx::ShaderHandle{ m_shaders[1].handle }).idx;
	}
	else if (ShaderProgramType::Compute == m_type || ShaderProgramType::VertexOnly == m_type)
	{
		m_programHandle = bgfx::createProgram(bgfx::ShaderHandle{ m_shaders[0].handle }).idx;
	}
	else
	{
		CD_WARN("Unknow shader program type of {}!", m_name);
	}

	if (!bgfx::isValid(bgfx::ProgramHandle{ m_programHandle }))
	{
		return false;
	}

	return true;
}

void ShaderResource::ClearShaderData(size_t index)
{
	m_shaders[index].binBlob.clear();
}

void ShaderResource::FreeShaderData(size_t index)
{
	ClearShaderData(index);
	ShaderBlob().swap(m_shaders[index].binBlob);
}

void ShaderResource::DistoryShaderHandle(size_t index)
{
	if (bgfx::isValid(bgfx::ShaderHandle{ m_shaders[index].handle }))
	{
		bgfx::destroy(bgfx::ShaderHandle{ m_shaders[index].handle });
		m_shaders[index].handle = bgfx::kInvalidHandle;
	}
}

void ShaderResource::DistoryProgramHandle()
{
	if (bgfx::isValid(bgfx::ProgramHandle{ m_programHandle }))
	{
		bgfx::destroy(bgfx::ProgramHandle{ m_programHandle });
		m_programHandle = bgfx::kInvalidHandle;
	}
}

void ShaderResource::CheckIndex(size_t index) const
{
	if (ShaderProgramType::Standard == m_type)
	{
		assert(index <= 1);
	}
	else
	{
		assert(index == 0);
	}
}

}