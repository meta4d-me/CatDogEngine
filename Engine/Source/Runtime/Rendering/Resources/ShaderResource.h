#pragma once

#include "Base/Template.h"
#include "IResource.h"
#include "Rendering/ShaderType.h"

#include <string>
#include <vector>

namespace engine
{

class ShaderResource : public IResource
{
public:
	using ShaderBlob = std::vector<std::byte>;

	struct ShaderInfo
	{
		ShaderType type;
		// Name without shader frature.
		std::string name;
		std::string scPath;
		// Should be the path to a specific variant file.
		std::string binPath;

		ShaderBlob binBlob;
		uint16_t handle = UINT16_MAX;
	};

public:
	ShaderResource() = default;
	ShaderResource(const ShaderResource&) = default;
	ShaderResource& operator=(const ShaderResource&) = default;
	ShaderResource(ShaderResource&&) = default;
	ShaderResource& operator=(ShaderResource&&) = default;
	virtual ~ShaderResource();

	virtual void Update() override;
	virtual void Reset() override;

	void SetShaders(const std::string& vsName, const std::string& fsName, const std::string& combine = "");
	void SetShader(ShaderType type, const std::string& name, const std::string& combine = "");

	void SetShaderInfo(ShaderInfo info, size_t index);
	ShaderInfo& GetShaderInfo(size_t index);
	const ShaderInfo& GetShaderInfo(size_t index) const;

	void SetActive(bool active) { m_active = active; }
	bool IsActive() const { return m_active; }

	void SetName(std::string name) { m_name = cd::MoveTemp(name); }
	std::string& GetName() { return m_name; }
	const std::string& GetName() const { return m_name; }

	ShaderProgramType GetType() const { return m_type; }
	void SetType(ShaderProgramType type) { m_type = type; }

	void SetFeaturesCombine(std::string combine) { m_featuresCombine = cd::MoveTemp(combine); }
	std::string& GetFeaturesCombine() { return m_featuresCombine; }
	const std::string& GetFeaturesCombine() const { return m_featuresCombine; }

	uint16_t GetHandle() const { return m_programHandle; }

private:
	bool LoadShader();
	bool BuildShaderHandle();
	bool BuildProgramHandle();

	void ClearShaderData(size_t index);
	void FreeShaderData(size_t index);
	void DistoryShaderHandle(size_t index);
	void DistoryProgramHandle();

	void CheckIndex(size_t index) const;

	ShaderInfo m_shaders[2];

	// Runtime
	bool m_active = false;
	std::string m_name;
	ShaderProgramType m_type = ShaderProgramType::None;
	std::string m_featuresCombine;
	uint32_t m_recycleCount = 0;

	// GPU
	uint16_t m_programHandle = UINT16_MAX;
};

}