#include "MainMenu.h"

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
			// It is not convenient in C++ to loop enum except define an extra array to wrap them.
			// C++ 20/23 ranges may look better but still needs std::iota inside its implementation.
			for (engine::ThemeColor theme = engine::ThemeColor::Black; theme < engine::ThemeColor::Count;
				theme = static_cast<engine::ThemeColor>(static_cast<int>(theme) + 1))
			{
				engine::ImGuiContextInstance* pImGuiContextInstance = GetImGuiContextInstance();
				if (ImGui::MenuItem(GetThemeColorName(theme), "", pImGuiContextInstance->GetImGuiThemeColor() == theme))
				{
					pImGuiContextInstance->SetImGuiThemeColor(theme);
				}
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu(CD_TEXT("TEXT_LANGUAGE")))
		{
			for (engine::Language language = engine::Language::ChineseSimplied; language < engine::Language::Count;
				 language = static_cast<engine::Language>(static_cast<int>(language) + 1))
			{
				engine::ImGuiContextInstance* pImGuiContextInstance = GetImGuiContextInstance();
				if (ImGui::MenuItem(GetLanguageName(language), "", pImGuiContextInstance->GetImGuiLanguage() == language))
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
	auto FrameEntities = [](engine::SceneWorld* pSceneWorld, const std::vector<engine::Entity>& entities)
	{
		if (entities.empty())
		{
			return;
		}

		std::optional<cd::AABB> optAABB;
		for (auto entity : entities)
		{
			if (engine::StaticMeshComponent* pStaticMesh = pSceneWorld->GetStaticMeshComponent(entity))
			{
				cd::AABB meshAABB = pStaticMesh->GetAABB();
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

		engine::CameraComponent* pCameraComponent = pSceneWorld->GetCameraComponent(pSceneWorld->GetMainCameraEntity());
		pCameraComponent->FrameAll(optAABB.value());
	};

	if (ImGui::BeginMenu("View"))
	{
		if (ImGui::MenuItem("Frame All"))
		{
			engine::SceneWorld* pSceneWorld = GetSceneWorld();
			if (size_t meshCount = pSceneWorld->GetStaticMeshEntities().size(); meshCount > 0)
			{
				FrameEntities(pSceneWorld, pSceneWorld->GetStaticMeshEntities());
			}
		}

		if (ImGui::MenuItem("Frame Selection"))
		{
			engine::SceneWorld* pSceneWorld = GetSceneWorld();
			if (engine::Entity selectedEntity = pSceneWorld->GetSelectedEntity(); selectedEntity != engine::INVALID_ENTITY)
			{
				FrameEntities(pSceneWorld, { selectedEntity });
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

		if (ImGui::MenuItem(CD_TEXT("TEXT_REBUILD_NONUBER_SHADERS")))
		{
			std::string nonUberPath = CDENGINE_BUILTIN_SHADER_PATH;
			ShaderBuilder::BuildNonUberShader(nonUberPath + "shaders");
		}
		if (ImGui::MenuItem(CD_TEXT("TEXT_REBUILD_PBR_SHADERS")))
		{
			ShaderBuilder::BuildUberShader(pSceneWorld->GetPBRMaterialType());
		}
		if (ImGui::MenuItem(CD_TEXT("TEXT_REBUILD_ANIMATION_SHADERS")))
		{
			ShaderBuilder::BuildUberShader(pSceneWorld->GetAnimationMaterialType());
		}
		ResourceBuilder::Get().Update();

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