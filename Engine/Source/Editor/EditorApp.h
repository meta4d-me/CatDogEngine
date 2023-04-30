#pragma once

#include "Application/IApplication.h"

#include <memory>
#include <vector>

namespace engine
{
class CameraController;
class FirstPersonCameraController;
class FlybyCamera;
class ImGuiBaseLayer;
class ImGuiContextInstance;
class Window;
class RenderContext;
class Renderer;
class SceneWorld;

}

struct ImGuiContext;

namespace editor
{

class EditorImGuiViewport;
class SceneView;

class EditorApp final : public engine::IApplication
{
public:
	EditorApp();
	EditorApp(const EditorApp&) = delete;
	EditorApp& operator=(const EditorApp&) = delete;
	EditorApp(EditorApp&&) = default;
	EditorApp& operator=(EditorApp&&) = default;
	virtual ~EditorApp();

	virtual void Init(engine::EngineInitArgs initArgs) override;
	virtual bool Update(float deltaTime) override;
	virtual void Shutdown() override;

	engine::Window* GetWindow(size_t index) const;
	engine::Window* GetMainWindow() const;
	size_t AddWindow(std::unique_ptr<engine::Window> pWindow);

	void InitRenderContext(engine::GraphicsBackend backend);
	void InitRenderGraph();
	void InitShaderPrograms() const;
	void AddEditorRenderer(std::unique_ptr<engine::Renderer> pRenderer);
	void AddEngineRenderer(std::unique_ptr<engine::Renderer> pRenderer);

	void InitEditorImGuiContext(engine::Language language);
	void InitEngineImGuiContext(engine::Language language);
	void InitImGuiViewports(engine::RenderContext* pRenderContext);
	void RegisterImGuiUserData(engine::ImGuiContextInstance* pImGuiContext);

	void InitECWorld();

private:
	// Windows
	std::vector<std::unique_ptr<engine::Window>> m_pAllWindows;

	// ImGui
	std::unique_ptr<engine::ImGuiContextInstance> m_pEditorImGuiContext;
	std::unique_ptr<engine::ImGuiContextInstance> m_pEngineImGuiContext;
	std::unique_ptr<EditorImGuiViewport> m_pEditorImGuiViewport;

	// Scene
	std::unique_ptr<engine::SceneWorld> m_pSceneWorld;
	editor::SceneView* m_pSceneView;
	engine::Renderer* m_pSceneRenderer;
	engine::Renderer* m_pDebugRenderer;
	engine::Renderer* m_pPBRSkyRenderer;
	engine::Renderer* m_pIBLSkyRenderer;
	engine::Renderer* m_pDDGIRenderer;

	// Rendering
	std::unique_ptr<engine::RenderContext> m_pRenderContext;
	std::vector<std::unique_ptr<engine::Renderer>> m_pEditorRenderers;
	std::vector<std::unique_ptr<engine::Renderer>> m_pEngineRenderers;

	// Controllers for processing input events.
	std::unique_ptr<engine::FirstPersonCameraController> m_pCameraController;
	std::unique_ptr<engine::CameraController> m_pNewCameraController;
};

}