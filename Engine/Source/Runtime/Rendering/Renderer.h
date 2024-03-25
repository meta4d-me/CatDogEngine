#pragma once

#include "Core/StringCrc.h"

#include <cstdint>
#include <set>
#include <string>

namespace engine
{

class Camera;
class RenderContext;
class RenderTarget;
class ShaderResource;
class StaticMeshComponent;

class Renderer
{
public:
	Renderer() = delete;
	explicit Renderer(uint16_t viewID, RenderTarget* pRenderTarget = nullptr);
	Renderer(const Renderer&) = delete;
	Renderer& operator=(const Renderer&) = delete;
	Renderer(Renderer&&) = delete;
	Renderer& operator=(Renderer&&) = delete;
	virtual ~Renderer() = default;

	static void SetRenderContext(RenderContext* pRenderContext);
	static RenderContext* GetRenderContext();

	virtual void Init() = 0;
	virtual void UpdateView(const float* pViewMatrix, const float* pProjectionMatrix) = 0;
	virtual void Render(float deltaTime) = 0;

	uint16_t GetViewID() const { return m_viewID; }
	
	void UpdateViewRenderTarget();
	void SetRenderTarget(RenderTarget* pRenderTarget) { m_pRenderTarget = pRenderTarget; }
	const RenderTarget* GetRenderTarget() const { return m_pRenderTarget; }

	virtual void SetEnable(bool value) { m_isEnable = value; }
	virtual bool IsEnable() const { return m_isEnable; }

	void SubmitStaticMeshDrawCall(StaticMeshComponent* pMeshComponent, uint16_t viewID, uint16_t programHandle);
	void SubmitStaticMeshDrawCall(StaticMeshComponent* pMeshComponent, uint16_t viewID, StringCrc programHandleIndex);

public:
	static void ScreenSpaceQuad(const RenderTarget* pRenderTarget, bool _originBottomLeft = false, float _width = 1.0f, float _height = 1.0f);
	void AddDependentShaderResource(ShaderResource *shaderResource) { m_dependentShaderResources.insert(shaderResource); }

protected:
	bool m_isEnable = true;
	uint16_t m_viewID = 0;
	RenderTarget* m_pRenderTarget = nullptr;

	// TODO : Need a generic way to manage the Renderer's dependency on shader resources, improve it.
	std::set<ShaderResource*> m_dependentShaderResources;
};

}