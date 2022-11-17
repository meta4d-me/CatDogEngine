#pragma once

#include "Core/EngineDefines.h"

#include <vector>
#include <memory>

namespace engine
{

class CSharpBridge;
class FlybyCamera;
class FirstPersonCameraController;
class PlatformWindow;
class RenderContext;
class Renderer;

class Engine
{
public:
	ENGINE_API explicit Engine();
	ENGINE_API virtual ~Engine();

	Engine(const Engine&) = delete;
	Engine& operator=(const Engine&) = delete;
	Engine(Engine&&) = delete;
	Engine& operator=(Engine&&) = delete;

	//
	// Init all basic modules, other optional modules should init by hand. 
	//
	ENGINE_API void Init();

	//
	// Execute main loop
	//
	ENGINE_API void MainLoop();

	//
	// Shutdown all modules in order.
	//
	ENGINE_API void Shutdown();

	//////////////////////////////////////////////////////////////////
	// Basic modules
	//////////////////////////////////////////////////////////////////
	// RenderContext includes Graphics, Renderers for different features
	RenderContext* GetRenderContext() const { return m_pRenderContext.get(); }

	// In editor mode, engine runs without windows so it is also optional.
	PlatformWindow* GetPlatformWindow() const { return m_pPlatformWindow.get(); }
	ENGINE_API void InitPlatformWindow(const char* pTitle, uint16_t width, uint16_t height);

private:
	std::unique_ptr<RenderContext> m_pRenderContext;
	std::unique_ptr<PlatformWindow> m_pPlatformWindow;
	std::unique_ptr<FlybyCamera> m_pFlybyCamera;
	std::unique_ptr<FirstPersonCameraController> m_pCameraController;

	std::vector<std::unique_ptr<Renderer>>  m_pRenderers;
};

}