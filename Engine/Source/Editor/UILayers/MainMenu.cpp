#include "MainMenu.h"

#include "EditorApp.h"
#include "ImGui/ImGuiContextInstance.h"
#include "ImGui/ThemeColor.h"
#include "Window/Window.h"
#include "Window/Input.h"
#include "Window/KeyCode.h"

#include <imgui/imgui.h>

namespace editor
{

MainMenu::~MainMenu()
{
}

void MainMenu::FileMenu()
{
	if (ImGui::BeginMenu("File"))
	{
		if (ImGui::MenuItem("New", "Ctrl N"))
		{
		}
		if (ImGui::MenuItem("Open", "Ctrl O"))
		{
		}
		if (ImGui::MenuItem("Open Recent"))
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
				pMainWindow->Closed();
			}
		}

		ImGui::EndMenu();
	}
}

void MainMenu::EditMenu()
{
	ImGuiIO& io = ImGui::GetIO();

	if (ImGui::BeginMenu("Edit"))
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

		if (ImGui::BeginMenu("Style"))
		{
			// It is not convenient in C++ to loop enum except define an extra array to wrap them.
			// C++ 20/23 ranges may look better but still needs std::iota inside its implementation.
			for (engine::ThemeColor theme = engine::ThemeColor::Black; theme < engine::ThemeColor::Count;
				theme = static_cast<engine::ThemeColor>(static_cast<int>(theme) + 1))
			{
				engine::ImGuiContextInstance* pImGuiContextInstance = reinterpret_cast<engine::ImGuiContextInstance*>(io.UserData);
				if (ImGui::MenuItem(GetThemeColorName(theme), "", pImGuiContextInstance->GetImGuiThemeColor() == theme))
				{
					pImGuiContextInstance->SetImGuiThemeColor(theme);
				}
			}

			ImGui::EndMenu();
		}

		ImGui::EndMenu();
	}
}

void MainMenu::WindowMenu()
{
	ImGuiIO& io = ImGui::GetIO();

	if (ImGui::BeginMenu("Window"))
	{
		engine::ImGuiContextInstance* pCurrentImguiContextInstance = reinterpret_cast<engine::ImGuiContextInstance*>(io.UserData);
		for (const auto& pDockableLayer : pCurrentImguiContextInstance->GetDockableLayers())
		{
			if (ImGui::MenuItem(pDockableLayer->GetName(), "", pDockableLayer->IsEnable()))
			{
				pDockableLayer->SetEnable(!pDockableLayer->IsEnable());
			}
		}

		ImGui::EndMenu();
	}
}

void MainMenu::AboutMenu()
{
	if (ImGui::BeginMenu("About"))
	{
		if (ImGui::MenuItem("Documents"))
		{
		}

		ImGui::EndMenu();
	}
}

void MainMenu::Init()
{

}

void MainMenu::Update()
{
	if (ImGui::BeginMainMenuBar())
	{
		FileMenu();
		EditMenu();
		WindowMenu();
		AboutMenu();
		ImGui::EndMainMenuBar();
	}

	if (engine::Input::Get().ContainsModifier(engine::KeyMod::KMOD_CTRL) 
		&& engine::Input::Get().IsKeyPressed(engine::KeyCode::q))
	{
		if (auto* pMainWindow = reinterpret_cast<engine::Window*>(ImGui::GetIO().BackendPlatformUserData))
		{ 
			pMainWindow->Closed();
		}
	}
}

}