#include "AssetBrowser.h"

#include "Consumers/CDConsumer/CDConsumer.h"
#include "ECWorld/ECWorldConsumer.h"
#include "ECWorld/MaterialComponent.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/StaticMeshComponent.h"
#include "ECWorld/World.h"
#include "Framework/Processor.h"
#include "Log/Log.h"
#include "Material/MaterialType.h"
#include "Producers/CDProducer/CDProducer.h"
#include "Producers/GenericProducer/GenericProducer.h"
#include "ImGui/IconFont/IconsMaterialDesignIcons.h"
#include "ImGui/ImGuiContextInstance.h"
#include "Rendering/WorldRenderer.h"
#include "Resources/ResourceBuilder.h"

#include <imgui/imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>

// This file depends on apis inside imgui.h so I placed here.
// TODO : OpenSource implementations about ImGui FileDialog are not ideal...
// We will replace it with our own implementation when have time to improve UI.
#include "ImGui/imfilebrowser.h"

namespace
{

bool IsCubeMapInputFile(const char* pFileExtension)
{
	constexpr const char* pFileExtensions[] = {".dds", ".exr", ".hdr", ".ktx", ".tga"};
	constexpr const int fileExtensionsSize = sizeof(pFileExtensions) / sizeof(pFileExtensions[0]);

	for (int extensionIndex = 0; extensionIndex < fileExtensionsSize; ++extensionIndex)
	{
		if (0 == strcmp(pFileExtensions[extensionIndex], pFileExtension))
		{
			return true;
		}
	}

	return false;
}

bool IsShaderInputFile(const char* pFileExtension)
{
	constexpr const char* pFileExtensions[] = { ".sc" };
	constexpr const int fileExtensionsSize = sizeof(pFileExtensions) / sizeof(pFileExtensions[0]);

	for (int extensionIndex = 0; extensionIndex < fileExtensionsSize; ++extensionIndex)
	{
		if (0 == strcmp(pFileExtensions[extensionIndex], pFileExtension))
		{
			return true;
		}
	}

	return false;
}

bool IsModelInputFile(const char* pFileExtension)
{
	constexpr const char* pFileExtensions[] = { ".cdbin", ".dae", ".fbx", ".glb", ".gltf", ".md5mesh" };
	constexpr const int fileExtensionsSize = sizeof(pFileExtensions) / sizeof(pFileExtensions[0]);
	for (int extensionIndex = 0; extensionIndex < fileExtensionsSize; ++extensionIndex)
	{
		if (0 == strcmp(pFileExtensions[extensionIndex], pFileExtension))
		{
			return true;
		}
	}

	return false;
}

}

namespace editor
{

AssetBrowser::~AssetBrowser()
{
}

void AssetBrowser::Init()
{
	m_pImportFileBrowser = std::make_unique<ImGui::FileBrowser>();
	m_pExportFileBrowser = std::make_unique<ImGui::FileBrowser>();
}

void AssetBrowser::UpdateAssetFolderTree()
{
	ImGui::BeginChild("TopMenuButtons", ImVec2(0, ImGui::GetWindowHeight() * 0.25f));
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.16f, 0.16f, 0.21f, 1.0f));
	if (ImGui::Button(reinterpret_cast<const char*>(ICON_MDI_FILE_IMPORT " Import")))
	{
		ImGui::OpenPopup("ImportAssets");
	}

	ImGui::SameLine();

	if (ImGui::Button(reinterpret_cast<const char*>(ICON_MDI_FILE_EXPORT " Export")))
	{
		ImGui::OpenPopup("ExportAssets");
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

		else if (ImGui::Selectable("Shader"))
		{
			m_importingAssetType = ImportAssetType::Shader;
			m_pImportFileBrowser->SetTitle("ImportAssets - Shader");
			//m_pImportFileBrowser->SetTypeFilters({ ".sc" }); // ".hlsl"
			m_pImportFileBrowser->Open();
		}
		else if(ImGui::Selectable("Model")) {
			m_importingAssetType = ImportAssetType::Model;
			m_pImportFileBrowser->SetTitle("ImportAssets - Model");
			//m_pImportFileBrowser->SetTypeFilters({ ".fbx", ".gltf" }); // ".obj", ".dae", ".ogex"
			m_pImportFileBrowser->Open();
		}
		else if(ImGui::Selectable("DDGI Model")) {
			m_importingAssetType = ImportAssetType::DDGIModel;
			m_pImportFileBrowser->SetTitle("ImportAssets - DDGI Model");
			m_pImportFileBrowser->Open();
		}

		ImGui::EndPopup();
	}

	if (ImGui::BeginPopup("ExportAssets"))
	{
		if (ImGui::Selectable("SceneDatabase"))
		{
			m_exportingAssetType = ExportAssetType::SceneDatabase;

			// TODO : The file browser seems impossible to export file.
			// Here I will use the select file as file name of exported file name.
			m_pExportFileBrowser->SetTitle("ExportAssets - SceneDatabase");
			m_pExportFileBrowser->Open();
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
	CD_INFO("Importing asset file : {0}", pFilePath);
	// Unknown is used by outside window event such as DragAndDrop a file.
	if (ImportAssetType::Unknown == m_importingAssetType)
	{
		// We will deduce file's asset type by its file extension.
		std::filesystem::path inputFilePath(pFilePath);
		if (!inputFilePath.has_extension())
		{
			return;
		}

		std::filesystem::path fileExtension = inputFilePath.extension();
		std::string pFileExtension = fileExtension.generic_string();
		if (IsCubeMapInputFile(pFileExtension.c_str()))
		{
			m_importingAssetType = ImportAssetType::CubeMap;
		}
		else if (IsShaderInputFile(pFileExtension.c_str()))
		{
			m_importingAssetType = ImportAssetType::Shader;
		}
		else if (IsModelInputFile(pFileExtension.c_str()))
		{
			m_importingAssetType = ImportAssetType::Model;
		}
		else
		{
			// Still unknown, exit.
			CD_WARN("Unable to deduce import asset type by file extension : {0}", pFileExtension.c_str());
			return;
		}
	}

	if (ImportAssetType::Model == m_importingAssetType || ImportAssetType::DDGIModel == m_importingAssetType)
	{
		ImportModelFile(pFilePath);
	}
	else if (ImportAssetType::CubeMap == m_importingAssetType)
	{
		std::filesystem::path inputFilePath(pFilePath);
		std::string inputFileName = inputFilePath.stem().generic_string();
		std::string outputFilePath = CDENGINE_RESOURCES_ROOT_PATH;
		outputFilePath += "Textures/skybox/" + inputFileName;
		ResourceBuilder::Get().AddCubeMapBuildTask(pFilePath, outputFilePath.c_str());
		ResourceBuilder::Get().Update();
	}
	else if (ImportAssetType::Shader == m_importingAssetType)
	{
		std::filesystem::path inputFilePath(pFilePath);
		std::string inputFileName = inputFilePath.stem().generic_string();

		ShaderType shaderType;
		if (inputFileName.find("vs_") != std::string::npos)
		{
			shaderType = ShaderType::Vertex;
		}
		else if (inputFileName.find("fs_") != std::string::npos)
		{
			shaderType = ShaderType::Fragment;
		}
		else if (inputFileName.find("cs_") != std::string::npos)
		{
			shaderType = ShaderType::Compute;
		}
		else
		{
			CD_WARN("Unable to deduce import shader type by file name : {0}", inputFileName.c_str());
			return;
		}

		std::string outputFilePath = CDENGINE_RESOURCES_ROOT_PATH;
		outputFilePath += "Shaders/" + inputFileName + ".bin";
		ResourceBuilder::Get().AddShaderBuildTask(shaderType, pFilePath, outputFilePath.c_str());
		ResourceBuilder::Get().Update();
	}
}

// Translate different 3D model file formats to memory data.
void AssetBrowser::ImportModelFile(const char* pFilePath)
{
	ImGuiIO& io = ImGui::GetIO();
	engine::RenderContext* pCurrentRenderContext = reinterpret_cast<engine::RenderContext*>(io.BackendRendererUserData);
	engine::ImGuiContextInstance* pImGuiContextInstance = reinterpret_cast<engine::ImGuiContextInstance*>(io.UserData);
	engine::SceneWorld* pSceneWorld = pImGuiContextInstance->GetSceneWorld();
	ECWorldConsumer ecConsumer(pSceneWorld, pCurrentRenderContext);

	cd::SceneDatabase* pSceneDatabase = pSceneWorld->GetSceneDatabase();
	ecConsumer.SetSceneDatabaseIDs(pSceneDatabase->GetNodeCount(), pSceneDatabase->GetMeshCount());

	std::filesystem::path inputFilePath(pFilePath);
	std::filesystem::path inputFileExtension = inputFilePath.extension();
	if (0 == inputFileExtension.compare(".cdbin"))
	{
		cdtools::CDProducer cdProducer(pFilePath);
		cdtools::Processor processor(&cdProducer, &ecConsumer, pSceneDatabase);
		processor.SetFlattenSceneDatabaseEnable(true);
		processor.Run();
	}
	//else if (0 == inputFileExtension.compare(".fbx"))
	//{
	//
	//}
	else
	{
		cdtools::GenericProducer genericProducer(pFilePath);
		genericProducer.SetSceneDatabaseIDs(pSceneDatabase->GetNodeCount(), pSceneDatabase->GetMeshCount(),
			pSceneDatabase->GetMaterialCount(), pSceneDatabase->GetTextureCount(), pSceneDatabase->GetLightCount());
		genericProducer.ActivateBoundingBoxService();
		genericProducer.ActivateCleanUnusedService();
		genericProducer.ActivateTangentsSpaceService();
		genericProducer.ActivateTriangulateService();
		genericProducer.ActivateSimpleAnimationService();
		// genericProducer.ActivateFlattenHierarchyService();


		if(m_importingAssetType == ImportAssetType::DDGIModel)
		{
			ecConsumer.ActivateDDGIService();
		}

		cdtools::Processor processor(&genericProducer, &ecConsumer, pSceneDatabase);
		processor.SetFlattenSceneDatabaseEnable(true);
		processor.Run();
	}
}

void AssetBrowser::ExportAssetFile(const char* pFilePath)
{
	ImGuiIO& io = ImGui::GetIO();
	engine::ImGuiContextInstance* pImGuiContextInstance = reinterpret_cast<engine::ImGuiContextInstance*>(io.UserData);
	engine::SceneWorld* pSceneWorld = pImGuiContextInstance->GetSceneWorld();
	cd::SceneDatabase* pSceneDatabase = pSceneWorld->GetSceneDatabase();

	if (ExportAssetType::SceneDatabase == m_exportingAssetType)
	{
		// Export main camera entity to scene.
		engine::Entity mainCameraEntity = pSceneWorld->GetMainCameraEntity();
		const engine::CameraComponent* pCameraComponent = pSceneWorld->GetCameraComponent(mainCameraEntity);
		assert(pCameraComponent);
		std::string cameraName = "Untitled_Camera";
		if (const engine::NameComponent* pNameComponent = pSceneWorld->GetNameComponent(mainCameraEntity))
		{
			cameraName = pNameComponent->GetName();
		}

		cd::Camera camera(cd::CameraID(pSceneDatabase->GetCameraCount()), cameraName.c_str());
		camera.SetEye(pCameraComponent->GetEye());
		camera.SetLookAt(pCameraComponent->GetLookAt());
		camera.SetUp(pCameraComponent->GetUp());
		camera.SetNearPlane(pCameraComponent->GetNearPlane());
		camera.SetFarPlane(pCameraComponent->GetFarPlane());
		camera.SetAspect(pCameraComponent->GetAspect());
		camera.SetFov(pCameraComponent->GetFov());
		pSceneDatabase->AddCamera(cd::MoveTemp(camera));
		
		// Export light entities to scene.
		for (engine::Entity lightEntity : pSceneWorld->GetLightEntities())
		{
			const engine::LightComponent* pLightComponent = pSceneWorld->GetLightComponent(lightEntity);

			std::string lightName = "Untitled_Light";
			if (const engine::NameComponent* pNameComponent = pSceneWorld->GetNameComponent(lightEntity))
			{
				lightName = pNameComponent->GetName();
			}

			cd::Light light(cd::LightID(pSceneDatabase->GetLightCount()), pLightComponent->GetType());
			light.SetName(lightName.c_str());
			light.SetIntensity(pLightComponent->GetIntensity());
			light.SetRange(pLightComponent->GetRange());
			light.SetRadius(pLightComponent->GetRadius());
			light.SetWidth(pLightComponent->GetWidth());
			light.SetHeight(pLightComponent->GetHeight());
			light.SetAngleScale(pLightComponent->GetAngleScale());
			light.SetAngleOffset(pLightComponent->GetAngleOffset());
			light.SetPosition(pLightComponent->GetPosition());
			light.SetColor(pLightComponent->GetColor());
			light.SetDirection(pLightComponent->GetDirection());
			light.SetUp(pLightComponent->GetUp());
			pSceneDatabase->AddLight(cd::MoveTemp(light));
		}

		std::filesystem::path selectFilePath(pFilePath);
		std::filesystem::path outputFilePath = selectFilePath.replace_extension(".cdbin");

		cdtools::CDConsumer consumer(outputFilePath.string().c_str());
		consumer.SetExportMode(cdtools::ExportMode::PureBinary);

		cdtools::Processor processor(nullptr, &consumer, pSceneDatabase);
		processor.SetFlattenSceneDatabaseEnable(true);
		processor.Run();
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
		ImportAssetFile(m_pImportFileBrowser->GetSelected().string().c_str());
		m_pImportFileBrowser->ClearSelected();
	}

	m_pExportFileBrowser->Display();
	if (m_pExportFileBrowser->HasSelected())
	{
		ExportAssetFile(m_pExportFileBrowser->GetSelected().string().c_str());
		m_pExportFileBrowser->ClearSelected();
	}
}

}