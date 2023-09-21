#pragma once

#include <cstdint>

namespace engine
{

class Camera;
class RenderContext;
class RenderTarget;
class ShaderVariantCollections;

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
	// All registered shaders are compiled in the App::Init stage,
	// only need to write the logic of creating the GPU resources here.
	virtual void Warmup() = 0;
	// Retuen false skips this renderer at current frame.
	// If your shader for the renderer doesn't change anything at runtime,
	// moving the logic for creating GPU resources from CheckResources to Warmup is more recommended practice.
	virtual bool CheckResources();
	virtual void UpdateView(const float* pViewMatrix, const float* pProjectionMatrix) = 0;
	virtual void Render(float deltaTime) = 0;

	uint16_t GetViewID() const { return m_viewID; }
	
	void UpdateViewRenderTarget();
	void SetRenderTarget(RenderTarget* pRenderTarget) { m_pRenderTarget = pRenderTarget; }
	const RenderTarget* GetRenderTarget() const { return m_pRenderTarget; }

	virtual void SetEnable(bool value) { m_isEnable = value; }
	virtual bool IsEnable() const { return m_isEnable; }

public:
	static void ScreenSpaceQuad(const RenderTarget* pRenderTarget, bool _originBottomLeft = false, float _width = 1.0f, float _height = 1.0f);

protected:
	uint16_t m_viewID = 0;
	RenderTarget* m_pRenderTarget = nullptr;
	bool m_isEnable = true;
};

}