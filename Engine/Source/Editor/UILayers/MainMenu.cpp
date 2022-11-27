#include "MainMenu.h"

#include "EditorApp.h"
#include "EditorImGuiContext.h"
#include "Preferences/ThemeColor.h"

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
		}

		ImGui::EndMenu();
	}
}

void MainMenu::EditMenu()
{
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
			for (ThemeColor theme = ThemeColor::Black; theme < ThemeColor::Count;
				theme = static_cast<ThemeColor>(static_cast<int>(theme) + 1))
			{
				if (ImGui::MenuItem(GetThemeColorName(theme), "", m_pEditorApp->GetImGuiContext()->GetImGuiThemeColor() == theme))
				{
					m_pEditorApp->GetImGuiContext()->SetImGuiThemeColor(theme);
				}
			}

			ImGui::EndMenu();
		}

		ImGui::EndMenu();
	}
}

void MainMenu::WindowMenu()
{
	if (ImGui::BeginMenu("Window"))
	{
		for (const auto& pDockableLayer : m_pEditorApp->GetImGuiContext()->GetDockableLayers())
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
}

}