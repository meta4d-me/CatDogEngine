﻿#include "EditorApp.h"

#include "Application/Engine.h"
#include "Display/CameraController.h"
#include "ECWorld/SceneWorld.h"
#include "ImGui/EditorImGuiViewport.h"
#include "ImGui/ImGuiContextInstance.h"
#include "ImGui/Localization.h"
#include "ImGui/UILayers/DebugPanel.h"
#include "ImGui/UILayers/Profiler.h"
#include "Log/Log.h"
#include "Math/MeshGenerator.h"
#include "Path/Path.h"
#include "Rendering/AABBRenderer.h"
#include "Rendering/AnimationRenderer.h"
#include "Rendering/BlendShapeRenderer.h"
#include "Rendering/BlitRenderTargetPass.h"
#ifdef ENABLE_DDGI
#include "Rendering/DDGIRenderer.h"
#endif
#include "Rendering/WhiteModelRenderer.h"
#include "Rendering/WireframeRenderer.h"
#include "Rendering/ImGuiRenderer.h"
#include "Rendering/PBRSkyRenderer.h"
#include "Rendering/BloomRenderer.h"
#include "Rendering/PostProcessRenderer.h"
#include "Rendering/RenderContext.h"
#include "Rendering/SkeletonRenderer.h"
#include "Rendering/SkyboxRenderer.h"
#include "Rendering/ShaderCollections.h"
#include "Rendering/TerrainRenderer.h"
#include "Rendering/WorldRenderer.h"
#include "Rendering/ParticleRenderer.h"
#include "Resources/FileWatcher.h"
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
#include "UILayers/SkeletonView.h"
#include "UILayers/Splash.h"
#include "UILayers/TestNodeEditor.h"
#include "Window/Input.h"
#include "Window/Window.h"

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
	if (!engine::Localization::ReadCSV(engine::Path::Join(CDEDITOR_RESOURCES_ROOT_PATH, "Text.csv")))
	{
		CD_ERROR("Failed to open CSV file");
	}

	// Phase 1 - Splash
	//		* Compile uber shader permutations automatically when initialization or detect changes
	//		* Show compile progresses so it still needs to update ui
	auto pSplashWindow = std::make_unique<engine::Window>("Loading", 500, 400);
	pSplashWindow->SetWindowIcon(m_initArgs.pIconFilePath);
	pSplashWindow->SetBordedLess(true);
	pSplashWindow->SetResizeable(false);

	// Init graphics backend
	InitRenderContext(m_initArgs.backend, pSplashWindow->GetNativeHandle());
	
	pSplashWindow->OnResize.Bind<engine::RenderContext, &engine::RenderContext::OnResize>(m_pRenderContext.get());
	AddWindow(cd::MoveTemp(pSplashWindow));

	InitEditorRenderers();
	EditorRenderersWarmup();
	InitEditorImGuiContext(m_initArgs.language);

	InitECWorld();
	m_pEditorImGuiContext->SetSceneWorld(m_pSceneWorld.get());

	InitEngineRenderers();

	// Add shader build tasks and create a thread to update tasks.
	InitShaderPrograms(initArgs.compileAllShaders);
	m_pEditorImGuiContext->AddStaticLayer(std::make_unique<Splash>("Splash"));

	std::thread resourceThread([]()
	{
		ResourceBuilder::Get().Update(true, false);
	});
	resourceThread.detach();

	InitFileWatcher();
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
	InitEditorController();

	// Add UI layers after finish imgui and rendering contexts' initialization.
	auto pMainMenu = std::make_unique<MainMenu>("MainMenu");
	pMainMenu->SetCameraController(m_pViewportCameraController.get());
	m_pEditorImGuiContext->AddStaticLayer(cd::MoveTemp(pMainMenu));

	auto pEntityList = std::make_unique<EntityList>("EntityList");
	pEntityList->SetCameraController(m_pViewportCameraController.get());
	m_pEditorImGuiContext->AddDynamicLayer(cd::MoveTemp(pEntityList));

	//m_pEditorImGuiContext->AddDynamicLayer(std::make_unique<GameView>("GameView"));

	auto pSceneView = std::make_unique<SceneView>("SceneView");
	m_pSceneView = pSceneView.get();
	pSceneView->SetCameraController(m_pViewportCameraController.get());
	pSceneView->SetSceneRenderer(m_pSceneRenderer);
	pSceneView->SetWhiteModelRenderer(m_pWhiteModelRenderer);
	pSceneView->SetWireframeRenderer(m_pWireframeRenderer);
	pSceneView->SetAABBRenderer(m_pAABBRenderer);
	m_pEditorImGuiContext->AddDynamicLayer(cd::MoveTemp(pSceneView));

	m_pEditorImGuiContext->AddDynamicLayer(std::make_unique<SkeletonView>("SkeletonView"));
	m_pEditorImGuiContext->AddDynamicLayer(std::make_unique<engine::Profiler>("Profiler"));
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
	//m_pEngineImGuiContext->AddDynamicLayer(std::make_unique<TestNodeEditor>("TestNodeEditor"));
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
	InitMaterialType();

	InitEditorCameraEntity();

	InitSkyEntity();

#ifdef ENABLE_DDGI
	m_pSceneWorld->InitDDGISDK();
	InitDDGIEntity();
#endif
}

void EditorApp::InitMaterialType()
{
	constexpr const char* WorldProgram = "WorldProgram";
	constexpr const char* AnimationProgram = "AnimationProgram";
	constexpr const char* TerrainProgram = "TerrainProgram";

	constexpr engine::StringCrc WorldProgramCrc{ WorldProgram };
	constexpr engine::StringCrc AnimationProgramCrc{ AnimationProgram };
	constexpr engine::StringCrc TerrainProgramCrc{ TerrainProgram };

	m_pRenderContext->RegisterShaderProgram(WorldProgramCrc, { "vs_PBR", "fs_PBR" });
	m_pRenderContext->RegisterShaderProgram(AnimationProgramCrc, { "vs_animation", "fs_animation" });
	m_pRenderContext->RegisterShaderProgram(TerrainProgramCrc, { "vs_terrain", "fs_terrain" });

	m_pSceneWorld = std::make_unique<engine::SceneWorld>();
	m_pSceneWorld->CreatePBRMaterialType(WorldProgram, IsAtmosphericScatteringEnable());
	m_pSceneWorld->CreateAnimationMaterialType(AnimationProgram);
	m_pSceneWorld->CreateTerrainMaterialType(TerrainProgram);
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
	cameraComponent.SetExposure(1.0f);
	cameraComponent.SetGammaCorrection(0.45f);
	cameraComponent.SetToneMappingMode(cd::ToneMappingMode::ACES);
	cameraComponent.SetBloomDownSampleTImes(4);
	cameraComponent.SetBloomIntensity(1.0f);
	cameraComponent.SetLuminanceThreshold(1.0f);
	cameraComponent.SetBlurTimes(0);
	cameraComponent.SetBlurSize(0.0f);
	cameraComponent.SetBlurScaling(1);
	cameraComponent.SetBloomEnable(false);
	cameraComponent.SetBlurEnable(false);
	cameraComponent.BuildProjectMatrix();
	cameraComponent.BuildViewMatrix(cameraTransform);
}

void EditorApp::InitSkyEntity()
{
	engine::World* pWorld = m_pSceneWorld->GetWorld();

	engine::Entity skyEntity = pWorld->CreateEntity();
	m_pSceneWorld->SetSkyEntity(skyEntity);

	auto& nameComponent = pWorld->CreateComponent<engine::NameComponent>(skyEntity);
	nameComponent.SetName("Sky");

	auto& skyComponent = pWorld->CreateComponent<engine::SkyComponent>(skyEntity);
	if (IsAtmosphericScatteringEnable())
	{
		skyComponent.SetSunDirection(cd::Direction(-0.1f, -0.9f, 0.5f));
		skyComponent.SetHeightOffset(1.0f);
		skyComponent.SetShadowLength(0.1f);
		skyComponent.SetAtmophericScatteringEnable(true);
	}

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
	meshComponent.Submit();
}

#ifdef ENABLE_DDGI
void EditorApp::InitDDGIEntity()
{
	engine::World* pWorld = m_pSceneWorld->GetWorld();

	engine::Entity ddgiEntity = pWorld->CreateEntity();
	m_pSceneWorld->SetDDGIEntity(ddgiEntity);

	auto& nameComponent = pWorld->CreateComponent<engine::NameComponent>(ddgiEntity);
	nameComponent.SetName("DDGI");

	pWorld->CreateComponent<engine::DDGIComponent>(ddgiEntity);
}
#endif

void EditorApp::InitFileWatcher()
{
	m_pFileWatcher = std::make_unique<FileWatcher>();

	FileWatchInfo info;
	info.m_watchPath = engine::Path::Join(CDENGINE_BUILTIN_SHADER_PATH, "shaders");
	info.m_isrecursive = false;
	info.m_onModify.Bind<editor::EditorApp, &editor::EditorApp::OnShaderHotModifiedCallback>(this);
	m_pFileWatcher->Watch(cd::MoveTemp(info));
}

void EditorApp::OnShaderHotModifiedCallback(const char* rootDir, const char* filePath)
{
	if (GetMainWindow()->GetInputFocus() || engine::Path::GetExtension(filePath) != engine::Path::ShaderInputExtension)
	{
	    // Do nothing when window holds the focus.
	    // Do nothing when a non-shader file is detected.
	    return;
	}
	m_pRenderContext->CheckModifiedProgram(engine::Path::GetFileNameWithoutExtension(filePath));
}

void EditorApp::UpdateMaterials()
{
	for (engine::Entity entity : m_pSceneWorld->GetMaterialEntities())
	{
		engine::MaterialComponent* pMaterialComponent = m_pSceneWorld->GetMaterialComponent(entity);
		if (!pMaterialComponent)
		{
			continue;
		}

		const std::string& programName = pMaterialComponent->GetShaderProgramName();
		const std::string& featuresCombine = pMaterialComponent->GetFeaturesCombine();

		// New shader feature added, need to compile new variants.
		m_pRenderContext->CheckShaderProgram(programName, featuresCombine);

		// Shader source files have been modified, need to re-compile existing variants.
		if (m_crtInputFocus && !m_preInputFocus)
		{
			m_pRenderContext->OnShaderHotModified(programName, featuresCombine);
		}
	}

	if (m_crtInputFocus && !m_preInputFocus)
	{
		m_pRenderContext->ClearModifiedProgramNameCrcs();
	}
}

void EditorApp::CompileAndLoadShaders()
{
	// 1. Compile
	for (const auto& task : m_pRenderContext->GetShaderCompileTasks())
	{
		// TODO : callback function per task
		ShaderBuilder::BuildShader(m_pRenderContext.get(), task);
	}

	// 2. Load
	if (!m_pRenderContext->GetShaderCompileTasks().empty())
	{
		ResourceBuilder::Get().Update(true, false);

		for (auto& info : m_pRenderContext->GetShaderCompileTasks())
		{
			m_pRenderContext->UploadShaderProgram(info.m_programName, info.m_featuresCombine);
		}

		m_pRenderContext->ClearShaderCompileTasks();
	}
}

void EditorApp::InitRenderContext(engine::GraphicsBackend backend, void* hwnd)
{
	CD_INFO("Init graphics backend : {}", nameof::nameof_enum(backend));

	engine::Path::SetGraphicsBackend(backend);
	m_pRenderContext = std::make_unique<engine::RenderContext>();
	m_pRenderContext->Init(backend, hwnd);
	engine::Renderer::SetRenderContext(m_pRenderContext.get());

	m_pShaderCollections = std::make_unique<engine::ShaderCollections>();
	m_pRenderContext->SetShaderCollections(m_pShaderCollections.get());
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

	if (IsAtmosphericScatteringEnable())
	{
		auto pPBRSkyRenderer = std::make_unique<engine::PBRSkyRenderer>(m_pRenderContext->CreateView(), pSceneRenderTarget);
		m_pPBRSkyRenderer = pPBRSkyRenderer.get();
		pPBRSkyRenderer->SetSceneWorld(m_pSceneWorld.get());
		AddEngineRenderer(cd::MoveTemp(pPBRSkyRenderer));
	}

	auto pSceneRenderer = std::make_unique<engine::WorldRenderer>(m_pRenderContext->CreateView(), pSceneRenderTarget);
	m_pSceneRenderer = pSceneRenderer.get();
	pSceneRenderer->SetSceneWorld(m_pSceneWorld.get());
	AddEngineRenderer(cd::MoveTemp(pSceneRenderer));

	auto pBlendShapeRenderer = std::make_unique<engine::BlendShapeRenderer>(m_pRenderContext->CreateView(), pSceneRenderTarget);
	pBlendShapeRenderer->SetSceneWorld(m_pSceneWorld.get());
	AddEngineRenderer(cd::MoveTemp(pBlendShapeRenderer));

	auto pTerrainRenderer = std::make_unique<engine::TerrainRenderer>(m_pRenderContext->CreateView(), pSceneRenderTarget);
	m_pTerrainRenderer = pTerrainRenderer.get();
	pTerrainRenderer->SetSceneWorld(m_pSceneWorld.get());
	AddEngineRenderer(cd::MoveTemp(pTerrainRenderer));

	auto pSkeletonRenderer = std::make_unique<engine::SkeletonRenderer>(m_pRenderContext->CreateView(), pSceneRenderTarget);
	pSkeletonRenderer->SetSceneWorld(m_pSceneWorld.get());
	AddEngineRenderer(cd::MoveTemp(pSkeletonRenderer));

	auto pAnimationRenderer = std::make_unique<engine::AnimationRenderer>(m_pRenderContext->CreateView(), pSceneRenderTarget);
	pAnimationRenderer->SetSceneWorld(m_pSceneWorld.get());
	AddEngineRenderer(cd::MoveTemp(pAnimationRenderer));

	auto pWhiteModelRenderer = std::make_unique<engine::WhiteModelRenderer>(m_pRenderContext->CreateView(), pSceneRenderTarget);
	m_pWhiteModelRenderer = pWhiteModelRenderer.get();
	pWhiteModelRenderer->SetSceneWorld(m_pSceneWorld.get());
	pWhiteModelRenderer->SetEnable(false);
	AddEngineRenderer(cd::MoveTemp(pWhiteModelRenderer));

	auto pParticlerenderer = std::make_unique<engine::ParticleRenderer>(m_pRenderContext->CreateView(), pSceneRenderTarget);
	pParticlerenderer->SetSceneWorld(m_pSceneWorld.get());
	AddEngineRenderer(cd::MoveTemp(pParticlerenderer));

#ifdef ENABLE_DDGI
	auto pDDGIRenderer = std::make_unique<engine::DDGIRenderer>(m_pRenderContext->CreateView(), pSceneRenderTarget);
	pDDGIRenderer->SetSceneWorld(m_pSceneWorld.get());
	AddEngineRenderer(cd::MoveTemp(pDDGIRenderer));
#endif

	auto pAABBRenderer = std::make_unique<engine::AABBRenderer>(m_pRenderContext->CreateView(), pSceneRenderTarget);
	m_pAABBRenderer = pAABBRenderer.get();
	pAABBRenderer->SetSceneWorld(m_pSceneWorld.get());
	AddEngineRenderer(cd::MoveTemp(pAABBRenderer));

	auto pWireframeRenderer = std::make_unique<engine::WireframeRenderer>(m_pRenderContext->CreateView(), pSceneRenderTarget);
	m_pWireframeRenderer = pWireframeRenderer.get();
	pWireframeRenderer->SetSceneWorld(m_pSceneWorld.get());
	AddEngineRenderer(cd::MoveTemp(pWireframeRenderer));

	auto pBlitRTRenderPass = std::make_unique<engine::BlitRenderTargetPass>(m_pRenderContext->CreateView(), pSceneRenderTarget);
	AddEngineRenderer(cd::MoveTemp(pBlitRTRenderPass));

	auto pBloomRenderer = std::make_unique<engine::BloomRenderer>(m_pRenderContext->CreateView(), pSceneRenderTarget);
	pBloomRenderer->SetSceneWorld(m_pSceneWorld.get());
	pBloomRenderer->SetEnable(false);
	AddEngineRenderer(cd::MoveTemp(pBloomRenderer));

	// We can debug vertex/material/texture information by just output that to screen as fragmentColor.
	// But postprocess will bring unnecessary confusion. 
	auto pPostProcessRenderer = std::make_unique<engine::PostProcessRenderer>(m_pRenderContext->CreateView(), pSceneRenderTarget);
	pPostProcessRenderer->SetSceneWorld(m_pSceneWorld.get());
	AddEngineRenderer(cd::MoveTemp(pPostProcessRenderer));

	// Note that if you don't want to use ImGuiRenderer for engine, you should also disable EngineImGuiContext.
	AddEngineRenderer(std::make_unique<engine::ImGuiRenderer>(m_pRenderContext->CreateView(), pSceneRenderTarget));
}

void EditorApp::EditorRenderersWarmup()
{
	for (std::unique_ptr<engine::Renderer>& pRenderer : m_pEditorRenderers)
	{
		pRenderer->Warmup();
	}
}

void EditorApp::EngineRenderersWarmup()
{
	for (std::unique_ptr<engine::Renderer>& pRenderer : m_pEngineRenderers)
	{
		pRenderer->Warmup();
	}
}

bool EditorApp::IsAtmosphericScatteringEnable() const
{
	engine::GraphicsBackend backend = engine::Path::GetGraphicsBackend();

	return engine::GraphicsBackend::Vulkan != backend
		&& engine::GraphicsBackend::OpenGL != backend
		&& engine::GraphicsBackend::OpenGLES != backend;
}

void EditorApp::InitShaderPrograms(bool compileAllShaders) const
{
	ShaderBuilder::CompileRegisteredNonUberShader(m_pRenderContext.get());

	if (compileAllShaders)
	{
		ShaderBuilder::CompileUberShaderAllVariants(m_pRenderContext.get(), m_pSceneWorld->GetPBRMaterialType());

#ifdef ENABLE_DDGI
		ShaderBuilder::CompileUberShaderAllVariants(m_pRenderContext.get(), m_pSceneWorld->GetDDGIMaterialType());
#endif
	}
	else
	{
		ShaderBuilder::CompileRegisteredUberShader(m_pRenderContext.get(), m_pSceneWorld->GetPBRMaterialType());

#ifdef ENABLE_DDGI
		ShaderBuilder::CompileRegisteredUberShader(m_pRenderContext.get(), m_pSceneWorld->GetDDGIMaterialType());
#endif
	}
}

void EditorApp::InitEditorController()
{
	// Controller for Input events.
	m_pViewportCameraController = std::make_unique<engine::CameraController>(
		m_pSceneWorld.get(),
		12.0f /* horizontal sensitivity */,
		12.0f /* vertical sensitivity */);
	m_pViewportCameraController->CameraToController();
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
	if (!m_bInitEditor && ResourceBuilder::Get().IsIdle())
	{
		m_bInitEditor = true;

		EngineRenderersWarmup();

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

		m_pEditorImGuiContext->ClearUILayers();
		InitEditorUILayers();

		InitEngineImGuiContext(m_initArgs.language);
		m_pEngineImGuiContext->SetSceneWorld(m_pSceneWorld.get());

		InitEngineUILayers();
	}

	GetMainWindow()->Update();
	m_crtInputFocus = GetMainWindow()->GetInputFocus();
	m_pEditorImGuiContext->Update(deltaTime);
	m_pSceneWorld->Update();

	engine::CameraComponent* pMainCameraComponent = m_pSceneWorld->GetCameraComponent(m_pSceneWorld->GetMainCameraEntity());
	engine::TerrainComponent* pTerrainComponent = m_pSceneWorld->GetTerrainComponent(m_pSceneWorld->GetSelectedEntity());
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
		GetMainWindow()->SetMouseVisible(m_pSceneView->IsShowMouse(), m_pSceneView->GetMouseFixedPositionX(), m_pSceneView->GetMouseFixedPositionY());
		if (m_pViewportCameraController)
		{
			m_pViewportCameraController->Update(deltaTime);
		}
		// Do Screen Space Smoothing
		if (pTerrainComponent && m_pSceneView->IsTerrainEditMode() && engine::Input::Get().IsMouseLBPressed())
		{
			float screenSpaceX = 2.0f * static_cast<float>(engine::Input::Get().GetMousePositionX() - m_pSceneView->GetWindowPosX()) /
				m_pSceneView->GetRenderTarget()->GetWidth() - 1.0f;
			float screenSpaceY = 1.0f - 2.0f * static_cast<float>(engine::Input::Get().GetMousePositionY() - m_pSceneView->GetWindowPosY()) /
				m_pSceneView->GetRenderTarget()->GetHeight();

			engine::TransformComponent* pCameraTransformComponent = m_pSceneWorld->GetTransformComponent(m_pSceneWorld->GetMainCameraEntity());
			cd::Vec3f camPos = pCameraTransformComponent->GetTransform().GetTranslation();

			pTerrainComponent->ScreenSpaceSmooth(screenSpaceX, screenSpaceY, pMainCameraComponent->GetProjectionMatrix().Inverse(),
				pMainCameraComponent->GetViewMatrix().Inverse(), camPos);
		}

		m_pEngineImGuiContext->SetWindowPosOffset(m_pSceneView->GetWindowPosX(), m_pSceneView->GetWindowPosY());
		m_pEngineImGuiContext->Update(deltaTime);

		UpdateMaterials();
		CompileAndLoadShaders();

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

	m_preInputFocus = m_crtInputFocus;

	return !GetMainWindow()->ShouldClose();
}

}
