#pragma once

#include "Application/IApplication.h"

#include <memory>
#include <vector>
#include <span>

namespace engine
{

class CameraController;
class FlybyCamera;
class ImGuiBaseLayer;
class ImGuiContextInstance;
class Window;
class RenderContext;
class Renderer;
class ResourceContext;
class AABBRenderer;
class RenderTarget;
class SceneWorld;
class ShaderCollections;

}

struct ImGuiContext;

namespace editor
{

class EditorImGuiViewport;
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

	engine::Window* GetWindow(size_t index) const;
	engine::Window* GetMainWindow() const { return GetWindow(0); }
	size_t AddWindow(std::unique_ptr<engine::Window> pWindow);
	void RemoveWindow(size_t index);

	void InitRenderContext(engine::GraphicsBackend backend, void* hwnd = nullptr);

	void InitEditorRenderers();
	void InitEngineRenderers();

	void EditorRenderersWarmup();
	void EngineRenderersWarmup();

	void InitShaderPrograms(bool compileAllShaders = false) const;
	void AddEditorRenderer(std::unique_ptr<engine::Renderer> pRenderer);
	void AddEngineRenderer(std::unique_ptr<engine::Renderer> pRenderer);

	void InitEditorImGuiContext(engine::Language language);
	void InitEditorUILayers();
	void InitEngineImGuiContext(engine::Language language);
	void InitEngineUILayers();
	void InitImGuiViewports(engine::RenderContext* pRenderContext);
	void RegisterImGuiUserData(engine::ImGuiContextInstance* pImGuiContext);

	void InitECWorld();
	void InitMaterialType();
	void InitEditorController();

	bool IsAtmosphericScatteringEnable() const;

private:
	void InitEditorCameraEntity();
	void InitSkyEntity();

#ifdef ENABLE_DDGI
	void InitDDGIEntity();
#endif

	void InitFileWatcher();
	void OnShaderHotModifiedCallback(const char* rootDir, const char* filePath);

	void UpdateMaterials();
	void CompileAndLoadShaders();
	void OnShaderCompileFailed(uint32_t handle, std::span<const char> str);

	bool m_crtInputFocus = true;
	bool m_preInputFocus = true;

	bool m_bInitEditor = false;
	engine::EngineInitArgs m_initArgs;

	// Windows
	std::vector<std::unique_ptr<engine::Window>> m_pAllWindows;

	// ImGui
	std::unique_ptr<engine::ImGuiContextInstance> m_pEditorImGuiContext;
	std::unique_ptr<engine::ImGuiContextInstance> m_pEngineImGuiContext;
	std::unique_ptr<EditorImGuiViewport> m_pEditorImGuiViewport;

	// Scene
	std::unique_ptr<engine::SceneWorld> m_pSceneWorld;
	editor::SceneView* m_pSceneView = nullptr;
	engine::Renderer* m_pSceneRenderer = nullptr;
	engine::Renderer* m_pWhiteModelRenderer = nullptr;
	engine::Renderer* m_pWireframeRenderer = nullptr;
	engine::Renderer* m_pPBRSkyRenderer = nullptr;
	engine::Renderer* m_pIBLSkyRenderer = nullptr;
	engine::Renderer* m_pTerrainRenderer = nullptr;
	engine::Renderer* m_pAABBRenderer = nullptr;
	engine::Renderer* m_pShadowMapRenderer = nullptr;
	engine::Renderer* m_pCelluloidRenderer = nullptr;
	engine::Renderer* m_pOutLineRenderer = nullptr;

	// Rendering
	std::unique_ptr<engine::RenderContext> m_pRenderContext;
	std::unique_ptr<engine::ResourceContext> m_pResourceContext;
	std::unique_ptr<engine::ShaderCollections> m_pShaderCollections;

	std::vector<std::unique_ptr<engine::Renderer>> m_pEditorRenderers;
	std::vector<std::unique_ptr<engine::Renderer>> m_pEngineRenderers;

	// Controllers for processing input events.
	std::unique_ptr<engine::CameraController> m_pViewportCameraController;

	std::unique_ptr<FileWatcher> m_pFileWatcher;
};

}