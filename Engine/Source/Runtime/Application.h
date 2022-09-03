#pragma once

#include "IGame.h"

#include <memory>
#include <string>

namespace engine::Rendering
{

class SceneRenderer;
class SkyRenderer;
class RenderContext;
class UIRenderer;

}

namespace engine
{

//class PlatformWindow;

class Application
{
public:
	Application(std::unique_ptr<IGame> pGame);
	~Application();

	Application(const Application&) = delete;
	Application& operator=(const Application&) = delete;
	Application(Application&&) = delete;
	Application& operator=(Application&) = delete;

	/// <summary>
	/// Init
	/// </summary>
	/// <param name="pWindowHandle">
	///		In game mode, pWindowHandle should be a nullptr which indicates Application to create Window by itself.
	///		In editor mode, pWindowHandle should be Editor view's window handle.
	/// </param>
	void Init(void* pWindowHandle = nullptr);
	void Execute();
	void SetCurrentSceneDatabase(std::string filePath);

	bool ShouldClose();

private:
	std::unique_ptr<IGame>			m_pGame;
	//PlatformWindow*				m_pWindow = nullptr;

	Rendering::RenderContext*		m_pRenderContext = nullptr;
	Rendering::SceneRenderer*		m_pSceneRenderer = nullptr;
	Rendering::SkyRenderer*			m_pSkyRenderer = nullptr;
	Rendering::UIRenderer*			m_pUIRenderer = nullptr;
};

}