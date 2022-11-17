#pragma once

#include "EngineDefines.h"
#include "IApplication.h"

#include <vector>
#include <memory>

namespace engine
{

class Window;
class RenderContext;
class Renderer;

class Engine
{
public:
	explicit Engine(std::unique_ptr<IApplication> pApplication);
	virtual ~Engine();

	Engine(const Engine&) = delete;
	Engine& operator=(const Engine&) = delete;
	Engine(Engine&&) = delete;
	Engine& operator=(Engine&&) = delete;

	static ENGINE_API Engine* Create(std::unique_ptr<IApplication> pApplication);
	static ENGINE_API void Destroy(Engine* pEngine);

	//
	// Init all basic modules.
	//
	ENGINE_API void Init(EngineInitArgs args);

	//
	// Execute main loop.
	//
	ENGINE_API void Run();

	//
	// Shutdown all modules in order.
	//
	ENGINE_API void Shutdown();

	//////////////////////////////////////////////////////////////////
	// Basic modules
	//////////////////////////////////////////////////////////////////
	RenderContext* GetRenderContext() const { return m_pRenderContext.get(); }
	void InitRenderContext(uint16_t width, uint16_t height);

private:
	std::unique_ptr<IApplication> m_pApplication;
	std::unique_ptr<RenderContext> m_pRenderContext;
};

}