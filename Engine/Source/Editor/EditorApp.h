#pragma once

#include "Application/IApplication.h"

#include <map>
#include <memory>
#include <vector>

namespace engine
{

class AABBRenderer;
class EditorCameraController;
class FlybyCamera;
class ImGuiBaseLayer;
class ImGuiContextInstance;
class ImGuiContextManager;
class Window;
class WindowManager;
class RenderContext;
class Renderer;
class RenderTarget;
class SceneWorld;
class ShaderCollections;

}

struct ImGuiContext;

namespace editor
{

class FileWatcher;
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

	// Window Management
	void InitWindowManager();
	engine::Window* GetMainWindow() const { return m_pMainWindow; }
	engine::WindowManager* GetWindowManager() const { return m_pWindowManager.get(); }
	void HandleMouseMotionEvent(uint32_t windowID, int x, int y);

	// Rendering Management
	void InitRenderContext(engine::GraphicsBackend backend, void* hwnd = nullptr);
	void InitShaderPrograms(bool compileAllShaders = false) const;

	void InitEditorRenderers();
	void AddEditorRenderer(std::unique_ptr<engine::Renderer> pRenderer);
	void EditorRenderersWarmup();

	void InitEngineRenderers();
	void AddEngineRenderer(std::unique_ptr<engine::Renderer> pRenderer);
	void EngineRenderersWarmup();

	// ImGui Management
	void InitEditorImGuiContext(engine::Language language);
	void InitEditorUILayers();
	void InitEngineImGuiContext(engine::Language language);
	void InitEngineUILayers();

	// Scene World
	void InitECWorld();
	
	// Misc
	void InitEditorController();
	bool IsAtmosphericScatteringEnable() const;

private:
	void InitEditorCameraEntity();
	void InitSkyEntity();

#ifdef ENABLE_DDGI
	void InitDDGIEntity();
#endif

	void UpdateMaterials();
	void LazyCompileAndLoadShaders();

	bool m_bInitEditor = false;
	engine::EngineInitArgs m_initArgs;

	// Windows
	engine::Window* m_pMainWindow = nullptr;
	std::unique_ptr<engine::WindowManager> m_pWindowManager;

	// ImGui
	engine::ImGuiContextInstance* m_pEditorImGuiContext = nullptr;
	engine::ImGuiContextInstance* m_pEngineImGuiContext = nullptr;
	std::unique_ptr<engine::ImGuiContextManager> m_pImGuiContextManager;

	// Scene
	std::unique_ptr<engine::SceneWorld> m_pSceneWorld;

	// Rendering
	std::unique_ptr<engine::RenderContext> m_pRenderContext;
	std::unique_ptr<engine::ShaderCollections> m_pShaderCollections;
	std::vector<std::unique_ptr<engine::Renderer>> m_pEditorRenderers;
	std::vector<std::unique_ptr<engine::Renderer>> m_pEngineRenderers;
	editor::SceneView* m_pSceneView = nullptr;
	engine::Renderer* m_pSceneRenderer = nullptr;
	engine::Renderer* m_pWhiteModelRenderer = nullptr;
	engine::Renderer* m_pWireframeRenderer = nullptr;
	engine::Renderer* m_pPBRSkyRenderer = nullptr;
	engine::Renderer* m_pIBLSkyRenderer = nullptr;
	engine::Renderer* m_pTerrainRenderer = nullptr;
	engine::Renderer* m_pAABBRenderer = nullptr;

	// Controllers for processing input events.
	std::unique_ptr<engine::EditorCameraController> m_pViewportCameraController;

	std::unique_ptr<FileWatcher> m_pFileWatcher;
};

}