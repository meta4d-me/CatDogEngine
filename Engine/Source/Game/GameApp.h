#pragma once

#include "Application/IApplication.h"

#include <memory>
#include <vector>

namespace engine
{

class CameraController;
class FlybyCamera;
class ImGuiBaseLayer;
class ImGuiContextInstance;
class Window;
class RenderContext;
class Renderer;
class RenderTarget;
class SceneWorld;

}

struct ImGuiContext;

namespace editor
{

class EditorImGuiViewport;
class SceneView;

class GameApp final : public engine::IApplication
{
public:
	GameApp();
	GameApp(const GameApp&) = delete;
	GameApp& operator=(const GameApp&) = delete;
	GameApp(GameApp&&) = default;
	GameApp& operator=(GameApp&&) = default;
	virtual ~GameApp();

	virtual void Init(engine::EngineInitArgs initArgs) override;
	virtual bool Update(float deltaTime) override;
	virtual void Shutdown() override;

	engine::Window* GetWindow(size_t index) const;
	engine::Window* GetMainWindow() const { return GetWindow(0); }
	size_t AddWindow(std::unique_ptr<engine::Window> pWindow);
	void RemoveWindow(size_t index);

	void InitRenderContext(engine::GraphicsBackend backend, void* hwnd = nullptr);
	void InitEngineRenderers();
	void AddEngineRenderer(std::unique_ptr<engine::Renderer> pRenderer);

	void InitEngineImGuiContext(engine::Language language);
	void InitEngineUILayers();
	void RegisterImGuiUserData(engine::ImGuiContextInstance* pImGuiContext);

	void InitECWorld();
	void InitController();

	bool EnablePBRSky() const;

private:
	void InitDDGIEntity();
	void InitSkyEntity();

	engine::EngineInitArgs m_initArgs;

	// Windows
	std::vector<std::unique_ptr<engine::Window>> m_pAllWindows;

	// ImGui
	std::unique_ptr<engine::ImGuiContextInstance> m_pEngineImGuiContext;

	// Scene
	std::unique_ptr<engine::SceneWorld> m_pSceneWorld;
	engine::Renderer* m_pSceneRenderer = nullptr;
	engine::Renderer* m_pDebugRenderer = nullptr;
	engine::Renderer* m_pPBRSkyRenderer = nullptr;
	engine::Renderer* m_pIBLSkyRenderer = nullptr;

	// Rendering
	std::unique_ptr<engine::RenderContext> m_pRenderContext;
	std::vector<std::unique_ptr<engine::Renderer>> m_pEngineRenderers;

	// Controllers for processing input events.
	std::shared_ptr<engine::CameraController> m_pCameraController;
};

}