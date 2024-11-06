#include "MainMenu.h"

#include "Display/CameraController.h"
#include "ECWorld/SceneWorld.h"
#include "EditorApp.h"
#include "ImGui/ImGuiContextInstance.h"
#include "ImGui/Localization.h"
#include "ImGui/ThemeColor.h"
#include "Log/Log.h"
#include "Path/Path.h"
#include "Resources/ResourceBuilder.h"
#include "Resources/ShaderBuilder.h"
#include "Window/Window.h"
#include "Window/Input.h"
#include "Window/KeyCode.h"

#include <imgui/imgui.h>
#include "ImGui/imfilebrowser.h"
#include <filesystem>
#include <format>

namespace editor
{

MainMenu::~MainMenu()
{
}

void MainMenu::FileMenu()
{
	if (ImGui::BeginMenu(CD_TEXT("TEXT_FILE")))
	{
		if (ImGui::MenuItem(CD_TEXT("TEXT_NEW"), "Ctrl N"))
		{
			m_pCreatProjectDialog->SetTitle("Creat");
			m_pCreatProjectDialog->Open();
		}
		if (ImGui::MenuItem("Open", "Ctrl O"))
		{
		}
		if (ImGui::MenuItem(CD_TEXT("TEXT_OPEN_RECENT")))
		{
		}

		ImGui::Separator();

		if (ImGui::MenuItem("Save", "Ctrl S"))
		{
		}
		if (ImGui::MenuItem("Save As", "Shift Ctrl S"))
		{
		}

		ImGui::Separator();

		if (ImGui::MenuItem("Quit", "Ctrl Q"))
		{
			if (auto* pMainWindow = reinterpret_cast<engine::Window*>(ImGui::GetIO().BackendPlatformUserData))
			{
				pMainWindow->Close();
			}
		}

		ImGui::EndMenu();
	}
}

void MainMenu::EditMenu()
{
	if (ImGui::BeginMenu(CD_TEXT("TEXT_EDIT")))
	{
		if (ImGui::MenuItem("Undo", "Ctrl Z"))
		{
		}
		if (ImGui::MenuItem("Redo", "Shift Ctrl Z"))
		{
		}

		ImGui::Separator();

		if (ImGui::MenuItem("Preference"))
		{
		}

		if (ImGui::BeginMenu(CD_TEXT("TEXT_STYLE")))
		{
			for (uint32_t index = 0U; index < nameof::enum_count<engine::ThemeColor>(); ++index)
			{
				engine::ThemeColor theme = static_cast<engine::ThemeColor>(index);
				engine::ImGuiContextInstance* pImGuiContextInstance = GetImGuiContextInstance();
				if (ImGui::MenuItem(nameof::nameof_enum(theme).data(), "", pImGuiContextInstance->GetImGuiThemeColor() == theme))
				{
					pImGuiContextInstance->SetImGuiThemeColor(theme);
				}
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu(CD_TEXT("TEXT_LANGUAGE")))
		{
			for (uint32_t index = 0U; index < nameof::enum_count<engine::Language>(); ++index)
			{
				engine::Language language = static_cast<engine::Language>(index);
				engine::ImGuiContextInstance* pImGuiContextInstance = GetImGuiContextInstance();
				if (ImGui::MenuItem(nameof::nameof_enum(language).data(), "", pImGuiContextInstance->GetImGuiLanguage() == language))
				{
					pImGuiContextInstance->SetImGuiLanguage(language);
				}
			}
			ImGui::EndMenu();
		}

		ImGui::EndMenu();
	}
}

void MainMenu::ViewMenu()
{
	auto FrameEntities = [](engine::SceneWorld* pSceneWorld, const std::vector<engine::Entity>& entities, engine::CameraController* pCameraController)
	{
		if (entities.empty())
		{
			return;
		}

		std::optional<cd::AABB> optAABB;
		for (auto entity : entities)
		{
			if (pSceneWorld->GetSkyComponent(entity))
			{
				continue;
			}

			if (auto* pCollisionMesh = pSceneWorld->GetCollisionMeshComponent(entity))
			{
				cd::AABB meshAABB = pCollisionMesh->GetAABB();
				if (engine::TransformComponent* pTransform = pSceneWorld->GetTransformComponent(entity))
				{
					meshAABB = meshAABB.Transform(pTransform->GetWorldMatrix());
				}

				if (optAABB.has_value())
				{
					optAABB.value().Merge(meshAABB);
				}
				else
				{
					optAABB = meshAABB;
				}
			}
		}

		if (optAABB.value().IsEmpty())
		{
			return;
		}

		engine::Entity mainCamera = pSceneWorld->GetMainCameraEntity();
		if (engine::CameraComponent* pCameraComponent = pSceneWorld->GetCameraComponent(mainCamera))
		{
			auto* pTransformComponent = pSceneWorld->GetTransformComponent(mainCamera);
			pCameraComponent->FrameAll(optAABB.value(), pTransformComponent->GetTransform());
			
			pTransformComponent->Dirty();
			pTransformComponent->Build();
			pCameraComponent->ViewDirty();
			pCameraComponent->BuildViewMatrix(pTransformComponent->GetTransform());

			// TODO : add event queue to get mouse down and up events.
			pCameraController->CameraToController();
		}
	};

	if (ImGui::BeginMenu("View"))
	{
		if (ImGui::MenuItem("Frame All"))
		{
			engine::SceneWorld* pSceneWorld = GetSceneWorld();
			if (size_t meshCount = pSceneWorld->GetStaticMeshEntities().size(); meshCount > 0)
			{
				FrameEntities(pSceneWorld, pSceneWorld->GetStaticMeshEntities(), m_pCameraController);
			}
		}

		if (ImGui::MenuItem("Frame Selection"))
		{
			engine::SceneWorld* pSceneWorld = GetSceneWorld();
			if (engine::Entity selectedEntity = pSceneWorld->GetSelectedEntity(); selectedEntity != engine::INVALID_ENTITY)
			{
				FrameEntities(pSceneWorld, { selectedEntity }, m_pCameraController);
			}
		}

		//if (ImGui::MenuItem("Frame Selection with Children"))
		//{
		//}

		ImGui::EndMenu();
	}
}

void MainMenu::WindowMenu()
{
	if (ImGui::BeginMenu(CD_TEXT("TEXT_WINDOW")))
	{
		for (const auto& pDockableLayer : GetImGuiContextInstance()->GetDockableLayers())
		{
			if (ImGui::MenuItem(pDockableLayer->GetName(), "", pDockableLayer->IsEnable()))
			{
				pDockableLayer->SetEnable(!pDockableLayer->IsEnable());
			}
		}

		ImGui::EndMenu();
	}
}

void MainMenu::BuildMenu()
{
	if (ImGui::BeginMenu(CD_TEXT("TEXT_BUILD")))
	{
		engine::SceneWorld* pSceneWorld = GetSceneWorld();

		if (ImGui::MenuItem(CD_TEXT("TEXT_BUILD_PBR_VARIANT")))
		{
			ShaderBuilder::RegisterUberShaderAllVariants(GetRenderContext(), pSceneWorld->GetPBRMaterialType());
			ResourceBuilder::Get().Update();
		}

#ifdef ENABLE_DDGI
		if (ImGui::MenuItem(CD_TEXT("TEXT_BUILD_PBR_VARIANT")))
		{
			ShaderBuilder::RegisterUberShaderAllVariants(m_pRenderContext.get(), m_pSceneWorld->GetDDGIMaterialType());
			ResourceBuilder::Get().Update();
		}
#endif

		ImGui::EndMenu();
	}
}

void MainMenu::AboutMenu()
{
	if (ImGui::BeginMenu(CD_TEXT("TEXT_ABOUT")))
	{
		if (ImGui::MenuItem(CD_TEXT("TEXT_DOCUMENTS")))
		{
		}

		ImGui::EndMenu();
	}
}

void MainMenu::Init()
{
	m_pCreatProjectDialog = std::make_unique<ImGui::FileBrowser>();
}

void MainMenu::Update()
{
	if (ImGui::BeginMainMenuBar())
	{
		FileMenu();
		EditMenu();
		ViewMenu();
		WindowMenu();
		BuildMenu();
		AboutMenu();
		ImGui::EndMainMenuBar();
	}

	m_pCreatProjectDialog->Display();

	if (engine::Input::Get().ContainsModifier(engine::KeyMod::KMOD_CTRL)
		&& engine::Input::Get().IsKeyPressed(engine::KeyCode::q))
	{
		if (auto* pMainWindow = reinterpret_cast<engine::Window*>(ImGui::GetIO().BackendPlatformUserData))
		{
			pMainWindow->Close();
		}
	}
}

}