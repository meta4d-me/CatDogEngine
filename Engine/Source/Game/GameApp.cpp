#include "GameApp.h"

#include "Application/Engine.h"
#include "Display/CameraController.h"
#include "ECWorld/SceneWorld.h"
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
#include "Scene/SceneDatabase.h"
#include "Window/Input.h"
#include "Window/Window.h"

#include <imgui/imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>

namespace editor
{

GameApp::GameApp()
{
}

GameApp::~GameApp()
{
}

void GameApp::Init(engine::EngineInitArgs initArgs)
{
	m_initArgs = cd::MoveTemp(initArgs);

	auto pMainWindow = std::make_unique<engine::Window>(initArgs.pTitle, initArgs.width, initArgs.height);
	pMainWindow->SetWindowIcon(m_initArgs.pIconFilePath);
	pMainWindow->SetResizeable(true);
	
	// Init graphics backend
	InitRenderContext(m_initArgs.backend, pMainWindow->GetNativeHandle());
	pMainWindow->OnResize.Bind<engine::RenderContext, &engine::RenderContext::OnResize>(m_pRenderContext.get());
	AddWindow(cd::MoveTemp(pMainWindow));
	m_pRenderContext->SetBackBufferSize(GetMainWindow()->GetWidth(), GetMainWindow()->GetHeight());

	InitECWorld();

	InitEngineRenderers();

	InitEngineImGuiContext(m_initArgs.language);
	m_pEngineImGuiContext->SetSceneWorld(m_pSceneWorld.get());

	InitEngineUILayers();
}

void GameApp::Shutdown()
{
}

engine::Window* GameApp::GetWindow(size_t index) const
{
	return m_pAllWindows[index].get();
}

size_t GameApp::AddWindow(std::unique_ptr<engine::Window> pWindow)
{
	size_t windowIndex = m_pAllWindows.size();
	m_pAllWindows.emplace_back(cd::MoveTemp(pWindow));
	return windowIndex;
}

void GameApp::RemoveWindow(size_t index)
{
	assert(index < m_pAllWindows.size());
	m_pAllWindows[index]->Close();
	m_pAllWindows.erase(m_pAllWindows.begin() + index);
}

void GameApp::InitEngineImGuiContext(engine::Language language)
{
	m_pEngineImGuiContext = std::make_unique<engine::ImGuiContextInstance>(GetMainWindow()->GetWidth(), GetMainWindow()->GetHeight());
	RegisterImGuiUserData(m_pEngineImGuiContext.get());

	std::vector<std::string> ttfFileNames = { "FanWunMing-SB.ttf" };
	m_pEngineImGuiContext->LoadFontFiles(ttfFileNames, language);

	m_pEngineImGuiContext->SetImGuiThemeColor(engine::ThemeColor::Light);
}

void GameApp::InitEngineUILayers()
{
	m_pEngineImGuiContext->AddDynamicLayer(std::make_unique<engine::DebugPanel>("DebugPanel"));
}

void GameApp::RegisterImGuiUserData(engine::ImGuiContextInstance* pImGuiContext)
{
	assert(GetMainWindow() && m_pRenderContext);

	ImGuiIO& io = ImGui::GetIO();
	assert(io.UserData == pImGuiContext);

	io.BackendPlatformUserData = GetMainWindow();
	io.BackendRendererUserData = m_pRenderContext.get();
}

void GameApp::InitECWorld()
{
	m_pSceneWorld = std::make_unique<engine::SceneWorld>();

	engine::World* pWorld = m_pSceneWorld->GetWorld();
	engine::Entity cameraEntity = pWorld->CreateEntity();
	m_pSceneWorld->SetMainCameraEntity(cameraEntity);
	auto& nameComponent = pWorld->CreateComponent<engine::NameComponent>(cameraEntity);
	nameComponent.SetName("MainCamera");

	m_pSceneWorld->InitDDGISDK();

	auto& cameraTransformComponent = pWorld->CreateComponent<engine::TransformComponent>(cameraEntity);
	cameraTransformComponent.SetTransform(cd::Transform::Identity());
	cameraTransformComponent.Build();
	auto& cameraTransform = cameraTransformComponent.GetTransform();
	auto& cameraComponent = pWorld->CreateComponent<engine::CameraComponent>(cameraEntity);
	cameraTransform.SetTranslation(cd::Point(0.0f, 0.0f, -100.0f));
	engine::CameraComponent::SetLookAt(cd::Direction(0.0f, 0.0f, 1.0f), cameraTransform);
	engine::CameraComponent::SetUp(cd::Direction(0.0f, 1.0f, 0.0f), cameraTransform);
	cameraComponent.SetAspect(1.0f);
	cameraComponent.SetFov(45.0f);
	cameraComponent.SetNearPlane(0.1f);
	cameraComponent.SetFarPlane(2000.0f);
	cameraComponent.SetNDCDepth(bgfx::getCaps()->homogeneousDepth ? cd::NDCDepth::MinusOneToOne : cd::NDCDepth::ZeroToOne);

	InitDDGIEntity();
	InitSkyEntity();
}

void GameApp::InitDDGIEntity()
{
	engine::World* pWorld = m_pSceneWorld->GetWorld();

	engine::Entity ddgiEntity = pWorld->CreateEntity();
	m_pSceneWorld->SetDDGIEntity(ddgiEntity);

	auto &nameComponent = pWorld->CreateComponent<engine::NameComponent>(ddgiEntity);
	nameComponent.SetName("DDGI");

	pWorld->CreateComponent<engine::DDGIComponent>(ddgiEntity);
}

void GameApp::InitSkyEntity()
{
	engine::World* pWorld = m_pSceneWorld->GetWorld();

	engine::Entity skyEntity = pWorld->CreateEntity();
	m_pSceneWorld->SetSkyEntity(skyEntity);

	auto &nameComponent = pWorld->CreateComponent<engine::NameComponent>(skyEntity);
	nameComponent.SetName("Sky");

	pWorld->CreateComponent<engine::SkyComponent>(skyEntity);

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

void GameApp::InitController()
{
	// Controller for Input events.
	m_pCameraController = std::make_shared<engine::CameraController>(
		m_pSceneWorld.get(),
		20.0f /* horizontal sensitivity */,
		20.0f /* vertical sensitivity */,
		160.0f /* Movement Speed*/);
	m_pCameraController->CameraToController();
}

void GameApp::InitRenderContext(engine::GraphicsBackend backend, void* hwnd)
{
	CD_INFO("Init graphics backend : {}", engine::GetGraphicsBackendName(backend));

	engine::Path::SetGraphicsBackend(backend);
	m_pRenderContext = std::make_unique<engine::RenderContext>();
	m_pRenderContext->Init(backend, hwnd);
	engine::Renderer::SetRenderContext(m_pRenderContext.get());
}

void GameApp::InitEngineRenderers()
{
	constexpr engine::StringCrc sceneViewRenderTargetName("SceneRenderTarget");
	std::vector<engine::AttachmentDescriptor> attachmentDesc = {
		{.textureFormat = engine::TextureFormat::RGBA32F },
		{.textureFormat = engine::TextureFormat::RGBA32F },
		{.textureFormat = engine::TextureFormat::D32F },
	};

	engine::RenderTarget* pSceneRenderTarget = nullptr;
	// pSceneRenderTarget = m_pRenderContext->CreateRenderTarget(sceneViewRenderTargetName,
	//	GetMainWindow()->GetWidth(), GetMainWindow()->GetHeight(), std::move(attachmentDesc));
	//
	//GetMainWindow()->OnResize.Bind<engine::RenderTarget, &engine::RenderTarget::Resize>(pSceneRenderTarget);

	if (EnablePBRSky())
	{
		auto pPBRSkyRenderer = std::make_unique<engine::PBRSkyRenderer>(m_pRenderContext->CreateView(), pSceneRenderTarget);
		m_pPBRSkyRenderer = pPBRSkyRenderer.get();
		pPBRSkyRenderer->SetSceneWorld(m_pSceneWorld.get());
		AddEngineRenderer(cd::MoveTemp(pPBRSkyRenderer));
	}
	else
	{
		auto pIBLSkyRenderer = std::make_unique<engine::SkyboxRenderer>(m_pRenderContext->CreateView(), pSceneRenderTarget);
		m_pIBLSkyRenderer = pIBLSkyRenderer.get();
		AddEngineRenderer(cd::MoveTemp(pIBLSkyRenderer));
	}

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

	//auto pBlitRTRenderPass = std::make_unique<engine::BlitRenderTargetPass>(m_pRenderContext->CreateView(), pSceneRenderTarget);
	//AddEngineRenderer(cd::MoveTemp(pBlitRTRenderPass));

	// We can debug vertex/material/texture information by just output that to screen as fragmentColor.
	// But postprocess will bring unnecessary confusion. 
	//auto pPostProcessRenderer = std::make_unique<engine::PostProcessRenderer>(m_pRenderContext->CreateView(), pSceneRenderTarget);
	//pPostProcessRenderer->SetSceneWorld(m_pSceneWorld.get());
	//AddEngineRenderer(cd::MoveTemp(pPostProcessRenderer));

	// Note that if you don't want to use ImGuiRenderer for engine, you should also disable EngineImGuiContext.
	AddEngineRenderer(std::make_unique<engine::ImGuiRenderer>(m_pRenderContext->CreateView(), pSceneRenderTarget));
}

bool GameApp::EnablePBRSky() const
{
	return engine::GraphicsBackend::OpenGL != engine::Path::GetGraphicsBackend() &&
		engine::GraphicsBackend::Vulkan != engine::Path::GetGraphicsBackend();
}

void GameApp::AddEngineRenderer(std::unique_ptr<engine::Renderer> pRenderer)
{
	pRenderer->Init();
	m_pEngineRenderers.emplace_back(cd::MoveTemp(pRenderer));
}

bool GameApp::Update(float deltaTime)
{
	GetMainWindow()->Update();
	m_pSceneWorld->Update();
	
	m_pRenderContext->BeginFrame();
	if (m_pEngineImGuiContext)
	{
		engine::CameraComponent* pMainCameraComponent = m_pSceneWorld->GetCameraComponent(m_pSceneWorld->GetMainCameraEntity());
		cd::Transform pMainCameraTranform = m_pSceneWorld->GetTransformComponent(m_pSceneWorld->GetMainCameraEntity())->GetTransform();

		assert(pMainCameraComponent);
		m_pCameraController->Update(deltaTime);
		pMainCameraComponent->BuildProject();

		m_pEngineImGuiContext->Update(deltaTime);
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
