#include "AssetBrowser.h"

#include "ImGui/IconFont/IconsMaterialDesignIcons.h"
#include "Rendering/SceneRenderer.h"

#include <imgui/imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>

// This file depends on apis inside imgui.h so I placed here.
// TODO : OpenSource implementations about ImGui FileDialog are not ideal...
// We will replace it with our own implementation when have time to improve UI.
#include "ImGui/imfilebrowser.h"

namespace editor
{

AssetBrowser::~AssetBrowser()
{
	if (m_pImportFileBrowser)
	{
		delete m_pImportFileBrowser;
		m_pImportFileBrowser = nullptr;
	}
}

void AssetBrowser::Init()
{
	m_pImportFileBrowser = new ImGui::FileBrowser();
	m_pImportFileBrowser->SetTitle("Asset - Import");
	// TODO : Import needs to have different options based on different kinds of file formats.
	// Model/Material/Texture/Audio/Script/...
	// /*".gltf", ".fbx"*/
	m_pImportFileBrowser->SetTypeFilters({ ".cdbin" });
}

void AssetBrowser::UpdateAssetFolderTree()
{
	ImGui::BeginChild("TopMenuButtons", ImVec2(0, ImGui::GetWindowHeight() * 0.25f));
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.16f, 0.16f, 0.21f, 1.0f));
	if (ImGui::Button(reinterpret_cast<const char*>(ICON_MDI_FILE_IMPORT " Import")))
	{
		m_pImportFileBrowser->Open();
	}
	ImGui::PopStyleColor();
	ImGui::EndChild();

	ImGui::BeginChild("FolderTree", ImVec2(ImGui::GetWindowWidth() * 0.5f, ImGui::GetWindowHeight() * 0.75f));


	ImGui::EndChild();
}

void AssetBrowser::UpdateAssetFileView()
{
	ImGui::BeginChild("FileView", ImVec2(ImGui::GetWindowWidth() * 0.5f, ImGui::GetWindowHeight() * 0.75f));

	ImGui::EndChild();
}

void AssetBrowser::Update()
{
	auto flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
	ImGui::Begin(GetName(), &m_isEnable, flags);

	// The first column is FolderTree view in the left.
	ImGui::BeginColumns("AssetBrowserColumns", 2);
	static bool bFirstUpdate = true;
	if (bFirstUpdate)
	{
		// Column border size is fixed in ImGui
		// https://stackoverflow.com/questions/72068188/how-do-i-change-the-border-size-for-a-table-in-dear-imgui

		// Colomn width doesn't have a flag to set only in the first frame.
		ImGui::SetColumnWidth(0, ImGui::GetWindowContentRegionMax().x * 0.4f);
		bFirstUpdate = false;
	}
	UpdateAssetFolderTree();

	// The next column is AssetFileView in the right.
	ImGui::NextColumn();
	UpdateAssetFileView();
	ImGui::EndColumns();

	ImGui::End();

	m_pImportFileBrowser->Display();
	if (m_pImportFileBrowser->HasSelected())
	{
		std::string importFilePath = m_pImportFileBrowser->GetSelected().string();
		m_pSceneRenderer->UpdateSceneDatabase(importFilePath);
		m_pImportFileBrowser->ClearSelected();
	}
}

}