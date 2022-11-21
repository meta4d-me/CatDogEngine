#include "MainMenu.h"

#include <imgui/imgui.h>

namespace editor
{

void FileMenu()
{
	if (ImGui::BeginMenu("File 文件"))
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

void EditMenu()
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

		ImGui::EndMenu();
	}
}

void WindowMenu()
{
	if (ImGui::BeginMenu("Window"))
	{
		if (ImGui::MenuItem("Asset Browser"))
		{
		}

		ImGui::EndMenu();
	}
}

void HelpMenu()
{
	if (ImGui::BeginMenu("Help"))
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

void MainMenu::BeginFrame()
{

}

void MainMenu::Update()
{
	//ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6, 6));
	//ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
	//ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 6));

	if (ImGui::BeginMenuBar())
	{
		FileMenu();
		EditMenu();
		WindowMenu();
		HelpMenu();
		ImGui::EndMenuBar();
	}


	//ImGui::PopStyleVar(3);
}

void MainMenu::EndFrame()
{

}

}