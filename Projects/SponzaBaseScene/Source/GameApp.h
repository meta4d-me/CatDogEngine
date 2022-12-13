#pragma once

#include "Application/IApplication.h"

#include <memory>
#include <vector>

namespace engine
{

class FirstPersonCameraController;
class FlybyCamera;
class ImGuiContextInstance;
class Window;
class RenderContext;
class Renderer;

}

namespace game
{

class GameApp final : public engine::IApplication
{
public:
	explicit GameApp();
	GameApp(const GameApp&) = delete;
	GameApp& operator=(const GameApp&) = delete;
	GameApp(GameApp&&) = default;
	GameApp& operator=(GameApp&&) = default;
	virtual ~GameApp();

	virtual void Init(engine::EngineInitArgs initArgs) override;
	virtual bool Update(float deltaTime) override;
	virtual void Shutdown() override;

	void AddRenderer(std::unique_ptr<engine::Renderer> pRenderer);

private:
	std::unique_ptr<engine::FlybyCamera> m_pFlybyCamera;
	std::unique_ptr<engine::FirstPersonCameraController> m_pCameraController;
	std::unique_ptr<engine::Window> m_pMainWindow;

	std::vector<std::unique_ptr<engine::Renderer>> m_pRenderers;

	std::unique_ptr<engine::RenderContext> m_pRenderContext;
	std::unique_ptr<engine::ImGuiContextInstance> m_pImGuiContext;
};

}