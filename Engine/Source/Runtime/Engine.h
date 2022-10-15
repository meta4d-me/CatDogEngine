#pragma once

#include "Core/EngineDefines.h"

#include <vector>

namespace engine
{

class CSharpBridge;
class PlatformWindow;
class Renderer;
class RenderContext;
class FlybyCamera;

class Engine
{
public:
	Engine() = default;
	Engine(const Engine&) = delete;
	Engine& operator=(const Engine&) = delete;
	Engine(Engine&&) = delete;
	Engine& operator=(Engine&&) = delete;
	~Engine();

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
	RenderContext* GetRenderContext() const { return m_pRenderContext; }

	//////////////////////////////////////////////////////////////////
	// Optional modules
	//////////////////////////////////////////////////////////////////
	// It will be useful when engine needs to communicate with C#.
	CSharpBridge* GetCSharpBridge() const { return m_pCSharpBridge; }
	ENGINE_API void InitCSharpBridge();

	// In editor mode, engine runs without windows so it is also optional.
	PlatformWindow* GetPlatformWindow() const { return m_pPlatformWindow; }
	ENGINE_API void InitPlatformWindow(const char* pTitle, uint16_t width, uint16_t height);

private:
	RenderContext*			m_pRenderContext = nullptr;
	CSharpBridge*			m_pCSharpBridge = nullptr;
	PlatformWindow*			m_pPlatformWindow = nullptr;
	FlybyCamera*			m_pFlybyCamera = nullptr;

	std::vector<Renderer*>  m_pRenderers;
};

}