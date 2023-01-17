#include "AssetBrowser.h"

#include "ECWorld/ECWorldConsumer.h"
#include "ECWorld/EditorSceneWorld.h"
#include "ECWorld/MaterialComponent.h"
#include "ECWorld/StaticMeshComponent.h"
#include "ECWorld/World.h"
#include "Framework/Processor.h"
#include "Material/MaterialType.h"
#include "Producers/GenericProducer/GenericProducer.h"
#include "ImGui/IconFont/IconsMaterialDesignIcons.h"
#include "Rendering/WorldRenderer.h"
#include "Resources/ResourceBuilder.h"

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
}

void AssetBrowser::UpdateAssetFolderTree()
{
	ImGui::BeginChild("TopMenuButtons", ImVec2(0, ImGui::GetWindowHeight() * 0.25f));
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.16f, 0.16f, 0.21f, 1.0f));
	if (ImGui::Button(reinterpret_cast<const char*>(ICON_MDI_FILE_IMPORT " Import")))
	{
		ImGui::OpenPopup("ImportAssets");
	}

	if (ImGui::BeginPopup("ImportAssets"))
	{
		if (ImGui::Selectable("Cubemap"))
		{
			m_importingAssetType = ImportAssetType::CubeMap;
			m_pImportFileBrowser->SetTitle("ImportAssets - Cubemap");
			//m_pImportFileBrowser->SetTypeFilters({ ".dds", "*.exr", "*.hdr", "*.ktx", ".tga" });
			m_pImportFileBrowser->Open();
		}
		else if (ImGui::Selectable("Model"))
		{
			m_importingAssetType = ImportAssetType::Model;
			m_pImportFileBrowser->SetTitle("ImportAssets - Model");
			//m_pImportFileBrowser->SetTypeFilters({ ".fbx", ".gltf" }); // ".obj", ".dae", ".ogex"
			m_pImportFileBrowser->Open();
		}
		else if (ImGui::Selectable("Shader"))
		{
			m_importingAssetType = ImportAssetType::Shader;
			m_pImportFileBrowser->SetTitle("ImportAssets - Shader");
			//m_pImportFileBrowser->SetTypeFilters({ ".sc" }); // ".hlsl"
			m_pImportFileBrowser->Open();
		}

		ImGui::EndPopup();
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

void AssetBrowser::ImportAssetFile(const char* pFilePath)
{
	if (ImportAssetType::Model == m_importingAssetType ||
		ImportAssetType::Unknown == m_importingAssetType)
	{
		ImGuiIO& io = ImGui::GetIO();
		engine::RenderContext* pCurrentRenderContext = reinterpret_cast<engine::RenderContext*>(io.BackendRendererUserData);

		// Translate different 3D model file formats to memory data.
		cd::SceneDatabase* pSceneDatabase = m_pEditorSceneWorld->GetSceneDatabase();
		cdtools::GenericProducer genericProducer(pFilePath);
		genericProducer.SetSceneDatabaseIDs(pSceneDatabase->GetNodeCount(), pSceneDatabase->GetMeshCount(),
			pSceneDatabase->GetMaterialCount(), pSceneDatabase->GetTextureCount(), pSceneDatabase->GetLightCount());
		genericProducer.ActivateBoundingBoxService();
		genericProducer.ActivateCleanUnusedService();
		genericProducer.ActivateTangentsSpaceService();
		genericProducer.ActivateTriangulateService();
		genericProducer.ActivateFlattenHierarchyService();

		engine::MaterialType pbrMaterialType = engine::MaterialType::GetPBRMaterialType();
		ECWorldConsumer ecConsumer(m_pEditorSceneWorld->GetWorld(), &pbrMaterialType, pCurrentRenderContext);
		ecConsumer.SetSceneDatabaseIDs(pSceneDatabase->GetNodeCount());
		cdtools::Processor processor(&genericProducer, &ecConsumer, pSceneDatabase);
		processor.Run();
	}
	else if (ImportAssetType::CubeMap == m_importingAssetType)
	{
		std::filesystem::path inputFilePath(pFilePath);
		std::string inputFileName = inputFilePath.stem().generic_string();
		std::string outputFilePath = CDENGINE_RESOURCES_ROOT_PATH;
		outputFilePath += "Textures/skybox/" + inputFileName;
		ResourceBuilder::Get().AddCubeMapBuildTask(pFilePath, inputFileName.c_str());
		ResourceBuilder::Get().Update();
	}
	else if (ImportAssetType::Shader == m_importingAssetType)
	{
		std::filesystem::path inputFilePath(pFilePath);
		std::string inputFileName = inputFilePath.stem().generic_string();
		std::string outputFilePath = CDENGINE_RESOURCES_ROOT_PATH;
		outputFilePath += "Shaders/" + inputFileName + ".bin";
		ResourceBuilder::Get().AddShaderBuildTask(pFilePath, outputFilePath.c_str());
		ResourceBuilder::Get().Update();
	}
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
		ImportAssetFile(m_pImportFileBrowser->GetSelected().generic_string().c_str());
		m_pImportFileBrowser->ClearSelected();
	}
}

}