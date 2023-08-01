#include "EditorApp.h"

#include "Application/Engine.h"
#include "Display/CameraController.h"
#include "ECWorld/SceneWorld.h"
#include "ImGui/EditorImGuiViewport.h"
#include "ImGui/ImGuiContextInstance.h"
#include "ImGui/Localization.h"
#include "ImGui/UILayers/DebugPanel.h"
#include "Log/Log.h"
#include "Math/MeshGenerator.h"
#include "Path/Path.h"
#include "Rendering/AnimationRenderer.h"
#include "Rendering/BlitRenderTargetPass.h"
#include "Rendering/DDGIRenderer.h"
#include "Rendering/DebugRenderer.h"
#include "Rendering/ImGuiRenderer.h"
#include "Rendering/PBRSkyRenderer.h"
#include "Rendering/PostProcessRenderer.h"
#include "Rendering/RenderContext.h"
#include "Rendering/SkyboxRenderer.h"
#include "Rendering/WorldRenderer.h"
#include "Resources/ResourceBuilder.h"
#include "Resources/ShaderBuilder.h"
#include "Scene/SceneDatabase.h"
#include "UILayers/AssetBrowser.h"
#include "UILayers/EntityList.h"
#include "UILayers/GameView.h"
#include "UILayers/ImGuizmoView.h"
#include "UILayers/Inspector.h"
#include "UILayers/MainMenu.h"
#include "UILayers/OutputLog.h"
#include "UILayers/SceneView.h"
#include "UILayers/Splash.h"
#include "Window/Input.h"
#include "Window/Window.h"

#ifdef ENABLE_TERRAIN_PRODUCER
#include "UILayers/TerrainEditor.h"
#include "Rendering/TerrainRenderer.h"
#endif

#include <imgui/imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>
#include "ImGui/imfilebrowser.h"

//#include <format>
#include <thread>

namespace editor
{

EditorApp::EditorApp()
{
}

EditorApp::~EditorApp()
{
}

void EditorApp::Init(engine::EngineInitArgs initArgs)
{
	m_initArgs = cd::MoveTemp(initArgs);

	// Load config files
	std::filesystem::path rootPath = CDEDITOR_RESOURCES_ROOT_PATH;
	rootPath /= "Text.csv";
	if (!engine::Localization::ReadCSV(rootPath.string()))
	{
		CD_ERROR("Failed to open CSV file");
	}


	// Phase 1 - Splash
	//		* Compile uber shader permutations automatically when initialization or detect changes
	//		* Show compile progresses so it still needs to update ui
	auto pSplashWindow = std::make_unique<engine::Window>("Splash", 500, 300);
	pSplashWindow->SetWindowIcon(m_initArgs.pIconFilePath);
	pSplashWindow->SetBordedLess(true);
	pSplashWindow->SetResizeable(false);
	
	// Init graphics backend
	InitRenderContext(m_initArgs.backend, pSplashWindow->GetNativeHandle());
	pSplashWindow->OnResize.Bind<engine::RenderContext, &engine::RenderContext::OnResize>(m_pRenderContext.get());
	AddWindow(cd::MoveTemp(pSplashWindow));

	InitEditorRenderers();
	InitEditorImGuiContext(m_initArgs.language);
	
	InitECWorld();
	m_pEditorImGuiContext->SetSceneWorld(m_pSceneWorld.get());

	// Add shader build tasks and create a thread to update tasks.
	InitShaderPrograms();
	m_pEditorImGuiContext->AddStaticLayer(std::make_unique<Splash>("Splash"));

	std::thread resourceThread([]()
	{
		ResourceBuilder::Get().Update();
	});
	resourceThread.detach();
}

void EditorApp::Shutdown()
{
}

engine::Window* EditorApp::GetWindow(size_t index) const
{
	return m_pAllWindows[index].get();
}

size_t EditorApp::AddWindow(std::unique_ptr<engine::Window> pWindow)
{
	size_t windowIndex = m_pAllWindows.size();
	m_pAllWindows.emplace_back(cd::MoveTemp(pWindow));
	return windowIndex;
}

void EditorApp::RemoveWindow(size_t index)
{
	assert(index < m_pAllWindows.size());
	m_pAllWindows[index]->Close();
	m_pAllWindows.erase(m_pAllWindows.begin() + index);
}

void EditorApp::InitEditorImGuiContext(engine::Language language)
{
	assert(GetMainWindow() && "Init window before imgui context");

	m_pEditorImGuiContext = std::make_unique<engine::ImGuiContextInstance>(GetMainWindow()->GetWidth(), GetMainWindow()->GetHeight(), true/*dockable*/);
	RegisterImGuiUserData(m_pEditorImGuiContext.get());

	// TODO : more font files to load and switch dynamically.
	std::vector<std::string> ttfFileNames = { "FanWunMing-SB.ttf" };
	m_pEditorImGuiContext->LoadFontFiles(ttfFileNames, language);

	// Bind event callbacks from current available input devices.
	GetMainWindow()->OnResize.Bind<engine::ImGuiContextInstance, &engine::ImGuiContextInstance::OnResize>(m_pEditorImGuiContext.get());

	// Set style settings.
	m_pEditorImGuiContext->SetImGuiThemeColor(engine::ThemeColor::Dark);
}

void EditorApp::InitEditorUILayers()
{
	// Add UI layers after finish imgui and rendering contexts' initialization.
	m_pEditorImGuiContext->AddStaticLayer(std::make_unique<MainMenu>("MainMenu"));

	auto pEntityList = std::make_unique<EntityList>("EntityList");
	pEntityList->SetCameraController(m_pCameraController);
	m_pEditorImGuiContext->AddDynamicLayer(cd::MoveTemp(pEntityList));

	//m_pEditorImGuiContext->AddDynamicLayer(std::make_unique<GameView>("GameView"));

	auto pSceneView = std::make_unique<SceneView>("SceneView");
	m_pSceneView = pSceneView.get();
	m_pEditorImGuiContext->AddDynamicLayer(cd::MoveTemp(pSceneView));

#ifdef ENABLE_TERRAIN_PRODUCER
	auto pTerrainEditor = std::make_unique<TerrainEditor>("Terrain Editor");
	m_pEditorImGuiContext->AddDynamicLayer(cd::MoveTemp(pTerrainEditor));
#endif

	m_pEditorImGuiContext->AddDynamicLayer(std::make_unique<Inspector>("Inspector"));

	auto pAssetBrowser = std::make_unique<AssetBrowser>("AssetBrowser");
	pAssetBrowser->SetSceneRenderer(m_pSceneRenderer);
	GetMainWindow()->OnDropFile.Bind<editor::AssetBrowser, &editor::AssetBrowser::ImportAssetFile>(pAssetBrowser.get());

	m_pEditorImGuiContext->AddDynamicLayer(cd::MoveTemp(pAssetBrowser));
	m_pEditorImGuiContext->AddDynamicLayer(std::make_unique<OutputLog>("OutputLog"));
}

void EditorApp::InitEngineImGuiContext(engine::Language language)
{
	constexpr engine::StringCrc sceneRenderTarget("SceneRenderTarget");
	engine::RenderTarget* pSceneRenderTarget = m_pRenderContext->GetRenderTarget(sceneRenderTarget);

	m_pEngineImGuiContext = std::make_unique<engine::ImGuiContextInstance>(pSceneRenderTarget->GetWidth(), pSceneRenderTarget->GetHeight());
	RegisterImGuiUserData(m_pEngineImGuiContext.get());

	std::vector<std::string> ttfFileNames = { "FanWunMing-SB.ttf" };
	m_pEngineImGuiContext->LoadFontFiles(ttfFileNames, language);

	m_pEngineImGuiContext->SetImGuiThemeColor(engine::ThemeColor::Light);

	pSceneRenderTarget->OnResize.Bind<engine::ImGuiContextInstance, &engine::ImGuiContextInstance::OnResize>(m_pEngineImGuiContext.get());
}

void EditorApp::InitEngineUILayers()
{
	//m_pEngineImGuiContext->AddDynamicLayer(std::make_unique<engine::DebugPanel>("DebugPanel"));
	auto pImGuizmoView = std::make_unique<editor::ImGuizmoView>("ImGuizmoView");
	pImGuizmoView->SetSceneView(m_pSceneView);
	m_pEngineImGuiContext->AddDynamicLayer(cd::MoveTemp(pImGuizmoView));
}

void EditorApp::InitImGuiViewports(engine::RenderContext* pRenderContext)
{
	if (ImGuiViewport* pMainViewport = ImGui::GetMainViewport())
	{
		pMainViewport->PlatformHandle = GetMainWindow();
		pMainViewport->PlatformHandleRaw = GetMainWindow()->GetNativeHandle();
	}
	m_pEditorImGuiViewport = std::make_unique<EditorImGuiViewport>(pRenderContext);
}

void EditorApp::RegisterImGuiUserData(engine::ImGuiContextInstance* pImGuiContext)
{
	assert(GetMainWindow() && m_pRenderContext);

	ImGuiIO& io = ImGui::GetIO();
	assert(io.UserData == pImGuiContext);

	io.BackendPlatformUserData = GetMainWindow();
	io.BackendRendererUserData = m_pRenderContext.get();
}

void EditorApp::InitECWorld()
{
	m_pSceneWorld = std::make_unique<engine::SceneWorld>();
	
	InitEditorCameraEntity();
	
	m_pSceneWorld->InitDDGISDK();
	InitDDGIEntity();

	InitSkyEntity();
}

void EditorApp::InitEditorCameraEntity()
{
	engine::World* pWorld = m_pSceneWorld->GetWorld();

	engine::Entity cameraEntity = pWorld->CreateEntity();
	m_pSceneWorld->SetMainCameraEntity(cameraEntity);
	auto& nameComponent = pWorld->CreateComponent<engine::NameComponent>(cameraEntity);
	nameComponent.SetName("MainCamera");
	
	auto& cameraTransformComponent = pWorld->CreateComponent<engine::TransformComponent>(cameraEntity);
	cameraTransformComponent.SetTransform(cd::Transform::Identity());
	cameraTransformComponent.Build();

	auto& cameraTransform = cameraTransformComponent.GetTransform();
	cameraTransform.SetTranslation(cd::Point(0.0f, 0.0f, -100.0f));
	engine::CameraComponent::SetLookAt(cd::Direction(0.0f, 0.0f, 1.0f), cameraTransform);
	engine::CameraComponent::SetUp(cd::Direction(0.0f, 1.0f, 0.0f), cameraTransform);

	auto& cameraComponent = pWorld->CreateComponent<engine::CameraComponent>(cameraEntity);
	cameraComponent.SetAspect(1.0f);
	cameraComponent.SetFov(45.0f);
	cameraComponent.SetNearPlane(0.1f);
	cameraComponent.SetFarPlane(2000.0f);
	cameraComponent.SetNDCDepth(bgfx::getCaps()->homogeneousDepth ? cd::NDCDepth::MinusOneToOne : cd::NDCDepth::ZeroToOne);
	cameraComponent.SetGammaCorrection(cd::Vec3f(0.45f));
	cameraComponent.BuildProjectMatrix();
	cameraComponent.BuildViewMatrix(cameraTransform);
}

void EditorApp::InitDDGIEntity()
{
	engine::World* pWorld = m_pSceneWorld->GetWorld();

	engine::Entity ddgiEntity = pWorld->CreateEntity();
	m_pSceneWorld->SetDDGIEntity(ddgiEntity);

	auto &nameComponent = pWorld->CreateComponent<engine::NameComponent>(ddgiEntity);
	nameComponent.SetName("DDGI");

	pWorld->CreateComponent<engine::DDGIComponent>(ddgiEntity);
}

void EditorApp::InitSkyEntity()
{
	engine::World* pWorld = m_pSceneWorld->GetWorld();

	engine::Entity skyEntity = pWorld->CreateEntity();
	m_pSceneWorld->SetSkyEntity(skyEntity);

	auto &nameComponent = pWorld->CreateComponent<engine::NameComponent>(skyEntity);
	nameComponent.SetName("Sky");

	auto& skyComponent = pWorld->CreateComponent<engine::SkyComponent>(skyEntity);

	cd::VertexFormat vertexFormat;
	vertexFormat.AddAttributeLayout(cd::VertexAttributeType::Position, cd::AttributeValueType::Float, 3);
	m_pRenderContext->CreateVertexLayout(engine::StringCrc("PosistionOnly"), vertexFormat.GetVertexLayout());

	cd::Box skyBox(cd::Point(-1.0f), cd::Point(1.0f));
	std::optional<cd::Mesh> optMesh = cd::MeshGenerator::Generate(skyBox, vertexFormat, false);
	assert(optMesh.has_value());

	auto& meshComponent = pWorld->CreateComponent<engine::StaticMeshComponent>(skyEntity);
	meshComponent.SetMeshData(&optMesh.value());
	meshComponent.SetRequiredVertexFormat(&vertexFormat);
	meshComponent.Build();

	auto& transformComponent = pWorld->CreateComponent<engine::TransformComponent>(skyEntity);
	transformComponent.SetTransform(cd::Transform(cd::Vec3f(0.0f, -1.0f, 0.0f), cd::Quaternion::Identity(), cd::Vec3f::One()));
	transformComponent.Build();
}

void EditorApp::InitRenderContext(engine::GraphicsBackend backend, void* hwnd)
{
	CD_INFO("Init graphics backend : {}", engine::GetGraphicsBackendName(backend));

	engine::Path::SetGraphicsBackend(backend);
	m_pRenderContext = std::make_unique<engine::RenderContext>();
	m_pRenderContext->Init(backend, hwnd);
	engine::Renderer::SetRenderContext(m_pRenderContext.get());
}

void EditorApp::InitEditorRenderers()
{
	AddEditorRenderer(std::make_unique<engine::ImGuiRenderer>(m_pRenderContext->CreateView()));
}

void EditorApp::InitEngineRenderers()
{
	constexpr engine::StringCrc sceneViewRenderTargetName("SceneRenderTarget");
	std::vector<engine::AttachmentDescriptor> attachmentDesc = {
		{.textureFormat = engine::TextureFormat::RGBA32F },
		{.textureFormat = engine::TextureFormat::RGBA32F },
		{.textureFormat = engine::TextureFormat::D32F },
	};

	// The init size doesn't make sense. It will resize by SceneView.
	engine::RenderTarget* pSceneRenderTarget = m_pRenderContext->CreateRenderTarget(sceneViewRenderTargetName, 1, 1, std::move(attachmentDesc));

	auto pSkyboxRenderer = std::make_unique<engine::SkyboxRenderer>(m_pRenderContext->CreateView(), pSceneRenderTarget);
	m_pIBLSkyRenderer = pSkyboxRenderer.get();
	pSkyboxRenderer->SetSceneWorld(m_pSceneWorld.get());
	AddEngineRenderer(cd::MoveTemp(pSkyboxRenderer));

	if (EnablePBRSky())
	{
		auto pPBRSkyRenderer = std::make_unique<engine::PBRSkyRenderer>(m_pRenderContext->CreateView(), pSceneRenderTarget);
		m_pPBRSkyRenderer = pPBRSkyRenderer.get();
		pPBRSkyRenderer->SetSceneWorld(m_pSceneWorld.get());
		AddEngineRenderer(cd::MoveTemp(pPBRSkyRenderer));
	}

#ifdef ENABLE_TERRAIN_PRODUCER
	auto pTerrainRenderer = std::make_unique<engine::TerrainRenderer>(m_pRenderContext->CreateView(), pSceneRenderTarget);
	pTerrainRenderer->SetSceneWorld(m_pSceneWorld.get());
	AddEngineRenderer(cd::MoveTemp(pTerrainRenderer));
#endif

	auto pSceneRenderer = std::make_unique<engine::WorldRenderer>(m_pRenderContext->CreateView(), pSceneRenderTarget);
	m_pSceneRenderer = pSceneRenderer.get();
	pSceneRenderer->SetSceneWorld(m_pSceneWorld.get());
	AddEngineRenderer(cd::MoveTemp(pSceneRenderer));

	auto pAnimationRenderer = std::make_unique<engine::AnimationRenderer>(m_pRenderContext->CreateView(), pSceneRenderTarget);
	pAnimationRenderer->SetSceneWorld(m_pSceneWorld.get());
	AddEngineRenderer(cd::MoveTemp(pAnimationRenderer));

	auto pDebugRenderer = std::make_unique<engine::DebugRenderer>(m_pRenderContext->CreateView(), pSceneRenderTarget);
	m_pDebugRenderer = pDebugRenderer.get();
	pDebugRenderer->SetEnable(false);
	pDebugRenderer->SetSceneWorld(m_pSceneWorld.get());
	AddEngineRenderer(cd::MoveTemp(pDebugRenderer));

	auto pDDGIRenderer = std::make_unique<engine::DDGIRenderer>(m_pRenderContext->CreateView(), pSceneRenderTarget);
	pDDGIRenderer->SetSceneWorld(m_pSceneWorld.get());
	AddEngineRenderer(cd::MoveTemp(pDDGIRenderer));

	auto pBlitRTRenderPass = std::make_unique<engine::BlitRenderTargetPass>(m_pRenderContext->CreateView(), pSceneRenderTarget);
	AddEngineRenderer(cd::MoveTemp(pBlitRTRenderPass));

	// We can debug vertex/material/texture information by just output that to screen as fragmentColor.
	// But postprocess will bring unnecessary confusion. 
	auto pPostProcessRenderer = std::make_unique<engine::PostProcessRenderer>(m_pRenderContext->CreateView(), pSceneRenderTarget);
	pPostProcessRenderer->SetSceneWorld(m_pSceneWorld.get());
	AddEngineRenderer(cd::MoveTemp(pPostProcessRenderer));

	// Note that if you don't want to use ImGuiRenderer for engine, you should also disable EngineImGuiContext.
	AddEngineRenderer(std::make_unique<engine::ImGuiRenderer>(m_pRenderContext->CreateView(), pSceneRenderTarget));
}

bool EditorApp::EnablePBRSky() const
{
	return engine::GraphicsBackend::OpenGL != engine::Path::GetGraphicsBackend() &&
		engine::GraphicsBackend::Vulkan != engine::Path::GetGraphicsBackend();
}

void EditorApp::InitShaderPrograms() const
{
	std::string nonUberBuildPath = CDENGINE_BUILTIN_SHADER_PATH;
	ShaderBuilder::BuildNonUberShader(nonUberBuildPath + "shaders");
	if (EnablePBRSky())
	{
		std::string atmBuildPath = CDENGINE_BUILTIN_SHADER_PATH;
		ShaderBuilder::BuildNonUberShader(atmBuildPath + "atm");
	}

	ShaderBuilder::BuildUberShader(m_pSceneWorld->GetPBRMaterialType());
	ShaderBuilder::BuildUberShader(m_pSceneWorld->GetAnimationMaterialType());
	ShaderBuilder::BuildUberShader(m_pSceneWorld->GetTerrainMaterialType());
	ShaderBuilder::BuildUberShader(m_pSceneWorld->GetDDGIMaterialType());
}

void EditorApp::InitController()
{
	// Controller for Input events.
	m_pCameraController = std::make_shared<engine::CameraController>(
		m_pSceneWorld.get(),
		20.0f /* horizontal sensitivity */,
		20.0f /* vertical sensitivity */,
		160.0f /* Movement Speed*/);
	m_pCameraController->CameraToController();
}

void EditorApp::AddEditorRenderer(std::unique_ptr<engine::Renderer> pRenderer)
{
	pRenderer->Init();
	m_pEditorRenderers.emplace_back(cd::MoveTemp(pRenderer));
}

void EditorApp::AddEngineRenderer(std::unique_ptr<engine::Renderer> pRenderer)
{
	pRenderer->Init();
	m_pEngineRenderers.emplace_back(cd::MoveTemp(pRenderer));
}

bool EditorApp::Update(float deltaTime)
{
	// TODO : it is better to remove these logics about splash -> editor switch here.
	// Better implementation is to have multiple Application or Window classes and they can switch.
	if (!m_bInitEditor)
	{
		if (0 == ResourceBuilder::Get().GetCurrentTaskCount())
		{
			m_bInitEditor = true;
			ShaderBuilder::UploadUberShader(m_pSceneWorld->GetPBRMaterialType());
			ShaderBuilder::UploadUberShader(m_pSceneWorld->GetAnimationMaterialType());
			ShaderBuilder::UploadUberShader(m_pSceneWorld->GetTerrainMaterialType());
			ShaderBuilder::UploadUberShader(m_pSceneWorld->GetDDGIMaterialType());

			// Phase 2 - Project Manager
			//		* TODO : Show project selector
			//GetMainWindow()->SetTitle("Project Manager");
			//GetMainWindow()->SetSize(800, 600);

			// Phase 3 - Editor
			//		* Load selected project to create assets, components, entities, ...s
			//		* Init engine renderers in SceneView to display visual results
			//		* Init editor ui layers
			//		* Init engine imgui context for ingame debug UI
			//		* Init engine ui layers
			GetMainWindow()->SetTitle(m_initArgs.pTitle);
			GetMainWindow()->SetSize(m_initArgs.width, m_initArgs.height);
			GetMainWindow()->SetFullScreen(m_initArgs.useFullScreen);
			GetMainWindow()->SetBordedLess(false);
			GetMainWindow()->SetResizeable(true);

			InitEngineRenderers();
			m_pEditorImGuiContext->ClearUILayers();
			InitController();
			InitEditorUILayers();

			InitEngineImGuiContext(m_initArgs.language);
			m_pEngineImGuiContext->SetSceneWorld(m_pSceneWorld.get());

			InitEngineUILayers();
		}
	}
	else
	{
		m_pCameraController->Update(deltaTime);
	}

	GetMainWindow()->Update();
	m_pSceneWorld->Update();
	m_pEditorImGuiContext->Update(deltaTime);

	engine::CameraComponent* pMainCameraComponent = m_pSceneWorld->GetCameraComponent(m_pSceneWorld->GetMainCameraEntity());
	assert(pMainCameraComponent);
	pMainCameraComponent->BuildProjectMatrix();

	m_pRenderContext->BeginFrame();
	for (std::unique_ptr<engine::Renderer>& pRenderer : m_pEditorRenderers)
	{
		if (pRenderer->IsEnable())
		{
			const float* pViewMatrix = nullptr;
			const float* pProjectionMatrix = nullptr;
			pRenderer->UpdateView(pViewMatrix, pProjectionMatrix);
			pRenderer->Render(deltaTime);
		}
	}

	if (m_pEngineImGuiContext)
	{
		m_pEngineImGuiContext->SetWindowPosOffset(m_pSceneView->GetWindowPosX(), m_pSceneView->GetWindowPosY());
		m_pEngineImGuiContext->Update(deltaTime);
		if (0 == m_pSceneView->GetDebugMode()) 
		{
			m_pSceneRenderer->SetEnable(true);
			m_pDebugRenderer->SetEnable(false);
		}else if(1 == m_pSceneView->GetDebugMode()) 
		{
			m_pSceneRenderer->SetEnable(false);
			m_pDebugRenderer->SetEnable(true);
		}
		for (std::unique_ptr<engine::Renderer>& pRenderer : m_pEngineRenderers)
		{
			if (pRenderer->IsEnable())
			{
				const float* pViewMatrix = pMainCameraComponent->GetViewMatrix().Begin();
				const float* pProjectionMatrix = pMainCameraComponent->GetProjectionMatrix().Begin();
				pRenderer->UpdateView(pViewMatrix, pProjectionMatrix);
				pRenderer->Render(deltaTime);
			}
		}
	}

	m_pRenderContext->EndFrame();

	engine::Input::Get().FlushInputs();
	
	return !GetMainWindow()->ShouldClose();
}

}
