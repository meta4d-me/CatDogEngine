#include "AssetBrowser.h"

#include "Consumers/CDConsumer/CDConsumer.h"
#include "ECWorld/ECWorldConsumer.h"
#include "ECWorld/MaterialComponent.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/StaticMeshComponent.h"
#include "ECWorld/World.h"
#include "Framework/Processor.h"
#include "ImGui/IconFont/IconsMaterialDesignIcons.h"
#include "ImGui/ImGuiContextInstance.h"
#include "ImGui/ImGuiUtils.hpp"
#include "Log/Log.h"
#include "Material/MaterialType.h"
#include "Producers/CDProducer/CDProducer.h"
#include "Producers/GenericProducer/GenericProducer.h"
#include "Rendering/WorldRenderer.h"
#include "Rendering/RenderContext.h"
#include "Resources/ResourceBuilder.h"
#include "Resources/ResourceLoader.h"

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
	constexpr const char* pFileExtensions[] = { ".dds", ".exr", ".hdr", ".ktx", ".tga" };
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
	constexpr const char* pFileExtensions[] = { ".cdbin", ".dae", ".fbx", ".glb", ".gltf", ".md5mesh", ".obj"};
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

std::string GetFilePathExtension(const std::string& FileName)
{
	auto pos = FileName.find_last_of('.');
	if (pos != std::string::npos)
		return FileName.substr(pos + 1);
	return "";
}

const char* GetIconFontIconc(const std::string& filePath)
{
	std::string extension = GetFilePathExtension(filePath);
	if (IsCubeMapInputFile(extension.data()))
	{
		return reinterpret_cast<const char*>(ICON_MDI_CUBE_UNFOLDED);
	}
	else if (IsModelInputFile(extension.data()))
	{
		return reinterpret_cast<const char*>(ICON_MDI_SHAPE);
	}
	else if (IsShaderInputFile(extension.data()))
	{
		return reinterpret_cast<const char*>(ICON_MDI_ALPHA_S_CIRCLE_OUTLINE);
	}

	return reinterpret_cast<const char*>(ICON_MDI_FILE);
}

void Tooltip(const char* text)
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5, 5));

	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::TextUnformatted(text);
		ImGui::EndTooltip();
	}

	ImGui::PopStyleVar();
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
	m_basePath = CDPROJECT_RESOURCES_ROOT_PATH;
	std::string baseDirectoryHandle = ProcessDirectory(std::filesystem::path(m_basePath), nullptr);
	m_baseProjectDirectory = m_directories[baseDirectoryHandle];
	m_currentDirectory = m_baseProjectDirectory;
}


bool AssetBrowser::RenderFile(int dirIndex, bool folder, int shownIndex, bool gridView)
{
	bool doubleClicked = false;
	std::filesystem::path resourcesPath = m_basePath;
	std::string extension = m_currentDirectory->Children[dirIndex]->FilePath.extension().generic_string();
	if (gridView)
	{
		ImGui::BeginGroup();
		const std::string fileName = m_currentDirectory->Children[dirIndex]->FilePath.filename().string();
		const std::string nameNoEx = m_currentDirectory->Children[dirIndex]->FilePath.filename().stem().string();
		std::string extension = m_currentDirectory->Children[dirIndex]->FilePath.extension().generic_string();
		if (folder)
		{
			ImGui::Button(reinterpret_cast<const char*>(ICON_MDI_FOLDER), ImVec2(m_gridSize, m_gridSize));
		}
		 if (!folder)
		{
			 if (IsTextureFile(extension))
			 {
				 std::filesystem::path texturesPath = resourcesPath / "Textures/" / "textures/" / fileName.c_str();
				 std::filesystem::path texviewPath = resourcesPath/ "Textures/" /"textures"/ (nameNoEx + ".dds");
				 ImGuiIO& io = ImGui::GetIO();
				 engine::RenderContext* pRenderContext = GetRenderContext();
				 engine::StringCrc textureCrc(nameNoEx);
				 bgfx::TextureHandle TextureHandle = pRenderContext->GetTexture(textureCrc);
				 if (!bgfx::isValid(TextureHandle))
				 {
					 ResourceBuilder::Get().AddTextureBuildTask(cd::MaterialTextureType::Normal, texturesPath.string().c_str(), texviewPath.string().c_str());
					 ResourceBuilder::Get().Update();
					 std::string texview = "Textures/textures/";
					 texview += (nameNoEx + ".dds");
					 bgfx::TextureHandle textureHandle = pRenderContext->CreateTexture(texview.c_str());
					 pRenderContext->SetTexture(textureCrc, textureHandle);
				 }
				 else
				 {
					 ImVec2 img_size(m_gridSize, m_gridSize);
					 ImGui::Image(ImTextureID(TextureHandle.idx), img_size);
				 }

			 }
			 if (0 == strcmp(".dds", extension.c_str()))
			 {
				 ImGui::Button(reinterpret_cast<const char*>(ICON_MDI_DELTA), ImVec2(m_gridSize, m_gridSize));
			 }
	
						
		}
		

		if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
		{
			doubleClicked = true;
		}

		auto newFname = StripExtras(fileName);

		ImGui::TextUnformatted(newFname.c_str());
		ImGui::EndGroup();

		if ((shownIndex + 1) % (m_gridItemPerRow ) != 0)
			ImGui::SameLine();
	}
	else
	{
		ImGui::TextUnformatted(folder ? reinterpret_cast<const char*>(ICON_MDI_FOLDER) : GetIconFontIconc(m_currentDirectory->Children[dirIndex]->FilePath.string()));
		ImGui::SameLine();
		if (ImGui::Selectable(m_currentDirectory->Children[dirIndex]->FilePath.filename().string().c_str(), false, ImGuiSelectableFlags_AllowDoubleClick))
		{
			if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				doubleClicked = true;
			}
		}
	}
	Tooltip(m_currentDirectory->Children[dirIndex]->FilePath.filename().string().c_str());
	if (doubleClicked)
	{
		if (folder)
		{
			m_previousDirectory = m_currentDirectory;
			m_currentDirectory = m_currentDirectory->Children[dirIndex];
			m_updateNavigationPath = true;
		}
		else
		{
			//TODO Put resource to scene if douleclicked
		}
	}

	return doubleClicked;
}

bool AssetBrowser::IsTextureFile(const std::string& extension)
{
	return 0 == strcmp(".png", extension.c_str()) ||
		   0 == strcmp(".tga", extension.c_str()) ||
		   0 == strcmp(".jpg", extension.c_str()) ||
		   0 == strcmp(".bmp", extension.c_str());
}

bool AssetBrowser::IsHiddenFile(const std::string& path)
{
	return path != ".." &&
		   path != "." &&
		   path[0] == '.';
}

void AssetBrowser::ChangeDirectory(std::shared_ptr<DirectoryInformation>& directory)
{
	if (!directory)
	{
		return;
	}

	m_previousDirectory		= m_currentDirectory;
	m_currentDirectory			= directory;
	m_updateNavigationPath  = true;
}

std::shared_ptr<DirectoryInformation> AssetBrowser::CreateDirectoryInfoSharedPtr(const std::filesystem::path& directoryPath, bool isDirectory)
{
	return std::make_shared<DirectoryInformation>(directoryPath, isDirectory);
}

void AssetBrowser::UpdateAssetFolderTree()
{
	ImGui::BeginChild("TopMenuButtons");
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
			m_importOptions.AssetType = ImportAssetType::CubeMap;
			m_pImportFileBrowser->SetTitle("ImportAssets - Cubemap");
			//m_pImportFileBrowser->SetTypeFilters({ ".dds", "*.exr", "*.hdr", "*.ktx", ".tga" });
			m_pImportFileBrowser->Open();
		}

		else if (ImGui::Selectable("Shader"))
		{
			m_importOptions.AssetType = ImportAssetType::Shader;
			m_pImportFileBrowser->SetTitle("ImportAssets - Shader");
			//m_pImportFileBrowser->SetTypeFilters({ ".sc" }); // ".hlsl"
			m_pImportFileBrowser->Open();
		}
		else if(ImGui::Selectable("Model"))
		{
			m_importOptions.AssetType = ImportAssetType::Model;
			m_pImportFileBrowser->SetTitle("ImportAssets - Model");
			//m_pImportFileBrowser->SetTypeFilters({ ".fbx", ".gltf" }); // ".obj", ".dae", ".ogex"
			m_pImportFileBrowser->Open();
		}
		else if(ImGui::Selectable("DDGI Model"))
		{
			m_importOptions.AssetType = ImportAssetType::DDGIModel;
			m_pImportFileBrowser->SetTitle("ImportAssets - DDGI Model");
			m_pImportFileBrowser->Open();
		}

		ImGui::EndPopup();
	}

	if (ImGui::BeginPopup("ExportAssets"))
	{
		if (ImGui::Selectable("SceneDatabase"))
		{
			m_exportOptions.AssetType = ExportAssetType::SceneDatabase;

			// TODO : The file browser seems impossible to export file.
			// Here I will use the select file as file name of exported file name.
			m_pExportFileBrowser->SetTitle("ExportAssets - SceneDatabase");
			m_pExportFileBrowser->Open();
		}

		ImGui::EndPopup();
	}

	ImGui::PopStyleColor();
	DrawFolder(m_baseProjectDirectory, true);
	ImGui::EndChild();
}

std::string AssetBrowser::StripExtras(const std::string& filename)
{
	std::vector<std::string> out;
	size_t start;
	size_t end = 0;

	while ((start = filename.find_first_not_of(".", end)) != std::string::npos)
	{
		end = filename.find(".", start);
		out.push_back(filename.substr(start, end - start));
	}

	int maxChars = static_cast<int>(m_gridSize / (ImGui::GetFontSize() * 0.5f));
	if (out[0].length() >= maxChars)
	{
		auto cutFilename = "     " + out[0].substr(0, maxChars - 3) + "...";
		return cutFilename;
	}

	return out[0];
}

void AssetBrowser::DrawFolder(const std::shared_ptr<DirectoryInformation>& dirInfo, bool defaultOpen)
{
	ImGuiTreeNodeFlags nodeFlags = ((dirInfo == m_currentDirectory) ? ImGuiTreeNodeFlags_Selected : 0);
	nodeFlags |= ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

	if (dirInfo->Parent == nullptr)
		nodeFlags |= ImGuiTreeNodeFlags_Framed;

	const ImColor TreeLineColor = ImColor(128, 128, 128, 128);
	const float SmallOffsetX = 6.0f; 
	ImDrawList* drawList = ImGui::GetWindowDrawList();

	if (!dirInfo->IsFile)
	{
		bool containsFolder = false;

		for (auto& file : dirInfo->Children)
		{
			if (!file->IsFile)
			{
				containsFolder = true;
				break;
			}
		}
		if (!containsFolder)
			nodeFlags |= ImGuiTreeNodeFlags_Leaf;
		if (defaultOpen)
			nodeFlags |= ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Leaf;

		nodeFlags |=ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_SpanAvailWidth;

		bool isOpen = ImGui::TreeNodeEx((void*)(intptr_t)(dirInfo.get()), nodeFlags, "");

		const char* folderIcon = reinterpret_cast<const char*>(((isOpen && containsFolder) || m_currentDirectory == dirInfo) ? ICON_MDI_FOLDER_OPEN : ICON_MDI_FOLDER);
		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
		ImGui::Text("%s ", folderIcon);
		ImGui::PopStyleColor();
		ImGui::SameLine();
		ImGui::TextUnformatted((const char*)dirInfo->FilePath.filename().string().c_str());

		ImVec2 verticalLineStart = ImGui::GetCursorScreenPos();

		if (ImGui::IsItemClicked())
		{
			m_previousDirectory = m_currentDirectory;
			m_currentDirectory = dirInfo;
			m_updateNavigationPath = true;
			std::filesystem::path a = m_currentDirectory->FilePath;
			std::string str = a.string();
			
		}

		if (isOpen && containsFolder)
		{
			verticalLineStart.x += SmallOffsetX; // to nicely line up with the arrow symbol
			ImVec2 verticalLineEnd = verticalLineStart;

			for (int i = 0; i < dirInfo->Children.size(); i++)
			{
				if (!dirInfo->Children[i]->IsFile)
				{
					auto currentPos = ImGui::GetCursorScreenPos();

					ImGui::Indent(10.0f);

					bool containsFolderTemp = false;
					for (auto& file : dirInfo->Children[i]->Children)
					{
						if (!file->IsFile)
						{
							containsFolderTemp = true;
							break;
						}
					}
					float HorizontalTreeLineSize = 16.0f;// chosen arbitrarily

					if (containsFolderTemp)
						HorizontalTreeLineSize *= 0.5f;
					DrawFolder(dirInfo->Children[i]);

					const ImRect childRect = ImRect(currentPos, currentPos + ImVec2(0.0f, ImGui::GetFontSize()));

					const float midpoint = (childRect.Min.y + childRect.Max.y) * 0.5f;
					drawList->AddLine(ImVec2(verticalLineStart.x, midpoint), ImVec2(verticalLineStart.x + HorizontalTreeLineSize, midpoint), TreeLineColor);
					verticalLineEnd.y = midpoint;

					ImGui::Unindent(10.0f);
				}
			}

			drawList->AddLine(verticalLineStart, verticalLineEnd, TreeLineColor);

			ImGui::TreePop();
		}

		if (isOpen && !containsFolder)
			ImGui::TreePop();
	}
}

std::string AssetBrowser::ProcessDirectory(const std::filesystem::path& directoryPath, const std::shared_ptr<DirectoryInformation>& parent)
{
	const auto& directory = m_directories[directoryPath.string()];
	if (directory)
	{
		return directory->FilePath.string();
	}

	std::shared_ptr<DirectoryInformation> directoryInfo = CreateDirectoryInfoSharedPtr(directoryPath, !std::filesystem::is_directory(directoryPath));
	directoryInfo->Parent = parent;
	directoryInfo->FilePath = directoryPath == m_basePath ? m_basePath : std::filesystem::relative(directoryPath, m_basePath);

	if (std::filesystem::is_directory(directoryPath))
	{
		for (auto entry : std::filesystem::directory_iterator(directoryPath))
		{
			if (!m_showHiddenFiles && IsHiddenFile(entry.path().string()))
			{
				continue;
			}

			std::string subdirHandle = ProcessDirectory(entry.path(), directoryInfo);
			directoryInfo->Children.push_back(m_directories[subdirHandle]);
		}
	}

	m_directories[directoryInfo->FilePath.string()] = directoryInfo;
	return directoryInfo->FilePath.string();
}
void AssetBrowser::UpdateAssetFileView()
{
	ImGui::BeginChild("FileView", ImVec2(0 , ImGui::GetWindowHeight() * 0.85f));
	{
		ImGui::BeginChild("directory_breadcrumbs", ImVec2(ImGui::GetColumnWidth(), ImGui::GetFrameHeightWithSpacing()));

		if (ImGui::Button(reinterpret_cast<const char*>(ICON_MDI_ARROW_LEFT)))
		{
			if (m_currentDirectory != m_baseProjectDirectory)
			{
				m_previousDirectory = m_currentDirectory;
				m_currentDirectory = m_currentDirectory->Parent;
				m_updateNavigationPath = true;
			}
		}

		ImGui::SameLine();
		if (ImGui::Button(reinterpret_cast<const char*>(ICON_MDI_ARROW_RIGHT)))
		{
			m_previousDirectory = m_currentDirectory;
			m_updateNavigationPath = true;
		}
		ImGui::SameLine();

		if (m_updateNavigationPath)
		{
			m_breadCrumbData.clear();
			auto current = m_currentDirectory;
			while (current)
			{
				if (current->Parent != nullptr)
				{
					m_breadCrumbData.push_back(current);
					current = current->Parent;
				}
				else
				{
					m_breadCrumbData.push_back(m_baseProjectDirectory);
					current = nullptr;
				}
			}

			std::reverse(m_breadCrumbData.begin(), m_breadCrumbData.end());
			m_updateNavigationPath = false;
		}

		if (m_isInListView)
		{
			if (ImGui::Button(reinterpret_cast<const char*>(ICON_MDI_VIEW_GRID)))
			{
				m_isInListView = !m_isInListView;
			}
			ImGui::SameLine();
		}
		else
		{
			if (ImGui::Button(reinterpret_cast<const char*>(ICON_MDI_VIEW_LIST)))
			{
				m_isInListView = !m_isInListView;
			}
			ImGui::SameLine();

		}

		ImGui::TextUnformatted(reinterpret_cast<const char*>(ICON_MDI_MAGNIFY));
		ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - ImGui::GetStyle().IndentSpacing);
		ImGui::EndChild();
	}

	{
		ImGui::BeginChild("##Scrolling", ImVec2(ImGui::GetWindowWidth() * 1.0f, ImGui::GetWindowHeight() * 0.75f));

		int shownIndex = 0;

		float xAvail = ImGui::GetContentRegionAvail().x * 0.812f;

		m_gridItemPerRow = (int)floor(xAvail / (m_gridSize + ImGui::GetStyle().ItemSpacing.x)) ; 
		m_gridItemPerRow = 1 > m_gridItemPerRow ? 1 : m_gridItemPerRow;

		for (int i = 0; i < m_currentDirectory->Children.size(); i++)
		{
			bool checkHiddenFiles = m_isInListView ? m_currentDirectory->Children.size() > 0 : true;
			if (checkHiddenFiles)
			{
				if (!m_showHiddenFiles && IsHiddenFile(m_currentDirectory->Children[i]->FilePath.filename().string()))
				{
					continue;
				}
			}

			bool doubleClicked = RenderFile(i, !m_currentDirectory->Children[i]->IsFile, shownIndex, !m_isInListView);

			if (doubleClicked)
			{
				break;
			}

			shownIndex++;
		}

		ImGui::EndChild();
	}

	ImGui::EndChild(); 
	ImGui::SliderFloat(" ", &m_gridSize, 40.0f, 160.0f, " ", ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_Logarithmic);
}

bool AssetBrowser::UpdateOptionDialog(const char* pTitle, bool& active, bool& importMesh, bool& importMaterial, bool& importTexture, bool& importCamera, bool& importLight)
{
	if (!active)
	{
		return false;
	}

	bool finish = false;

	ImGui::OpenPopup(pTitle);
	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopupModal(pTitle, nullptr, ImGuiWindowFlags_AlwaysVerticalScrollbar))
	{
		ImGui::SetWindowSize(ImVec2(400, 800));
		bool isMeshOpen = ImGui::CollapsingHeader("Mesh Options", ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_DefaultOpen);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
		ImGui::Separator();

		if (isMeshOpen)
		{
			ImGuiUtils::ImGuiProperty<bool>("Mesh", importMesh);
		}

		ImGui::Separator();
		ImGui::PopStyleVar();

		bool isMaterialOpen = ImGui::CollapsingHeader("Material Options", ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_DefaultOpen);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
		ImGui::Separator();
		if (isMaterialOpen)
		{
			ImGuiUtils::ImGuiProperty<bool>("Material", importMaterial);
			ImGuiUtils::ImGuiProperty<bool>("Texture", importTexture);
		}
		ImGui::Separator();
		ImGui::PopStyleVar();

		bool isOtherOpen = ImGui::CollapsingHeader("Other", ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_DefaultOpen);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
		ImGui::Separator();
		if (isOtherOpen)
		{
			ImGuiUtils::ImGuiProperty<bool>("Camera", importCamera);
			ImGuiUtils::ImGuiProperty<bool>("Light", importLight);
		}
		ImGui::Separator();
		ImGui::PopStyleVar();

		ImGui::Separator();

		auto CloseOptionDialog = [&]()
		{
			ImGui::CloseCurrentPopup();
			active = false;
		};

		if (ImGui::Button("OK", ImVec2(120, 0)))
		{
			finish = true;
			CloseOptionDialog();
		}

		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0)))
		{
			CloseOptionDialog();
		}
		ImGui::EndPopup();
	}

	return finish;
}

void AssetBrowser::ImportAssetFile(const char* pFilePath)
{
	CD_INFO("Importing asset file : {0}", pFilePath);
	// Unknown is used by outside window event such as DragAndDrop a file.
	if (ImportAssetType::Unknown == m_importOptions.AssetType)
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
			m_importOptions.AssetType = ImportAssetType::CubeMap;
		}
		else if (IsShaderInputFile(pFileExtension.c_str()))
		{
			m_importOptions.AssetType = ImportAssetType::Shader;
		}
		else if (IsModelInputFile(pFileExtension.c_str()))
		{
			m_importOptions.AssetType = ImportAssetType::Model;
		}
		else
		{
			// Still unknown, exit.
			CD_WARN("Unable to deduce import asset type by file extension : {0}", pFileExtension.c_str());
			return;
		}
	}

	if (ImportAssetType::Model == m_importOptions.AssetType || ImportAssetType::DDGIModel == m_importOptions.AssetType)
	{
		ImportModelFile(pFilePath);
	}
	else if (ImportAssetType::CubeMap == m_importOptions.AssetType)
	{
		std::filesystem::path inputFilePath(pFilePath);
		std::string inputFileName = inputFilePath.stem().generic_string();
		std::string outputFilePath = CDPROJECT_RESOURCES_ROOT_PATH;
		outputFilePath += "Textures/skybox/" + inputFileName;
		ResourceBuilder::Get().AddCubeMapBuildTask(pFilePath, outputFilePath.c_str());
		ResourceBuilder::Get().Update();
	}
	else if (ImportAssetType::Shader == m_importOptions.AssetType)
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

		std::string outputFilePath = CDPROJECT_RESOURCES_ROOT_PATH;
		outputFilePath += "Shaders/" + inputFileName + ".bin";
		ResourceBuilder::Get().AddShaderBuildTask(shaderType, pFilePath, outputFilePath.c_str());
		ResourceBuilder::Get().Update();
	}
}

void AssetBrowser::ProcessSceneDatabase(cd::SceneDatabase* pSceneDatabase, bool keepMesh, bool keepMaterial, bool keepTexture, bool keepCamera, bool keepLight)
{
	if (!keepMesh)
	{
		pSceneDatabase->GetMeshes().clear();
	}

	if (!keepMaterial)
	{
		pSceneDatabase->GetMaterials().clear();
		for (auto& mesh : pSceneDatabase->GetMeshes())
		{
			mesh.SetMaterialID(cd::MaterialID::InvalidID);
		}
	}

	if (!keepTexture)
	{
		pSceneDatabase->GetTextures().clear();
		for (auto& material : pSceneDatabase->GetMaterials())
		{
			for (int textureTypeIndex = 0; textureTypeIndex < static_cast<int>(cd::MaterialTextureType::Count); ++textureTypeIndex)
			{
				material.RemoveTexture(static_cast<cd::MaterialTextureType>(textureTypeIndex));
			}
		}
	}

	if (!keepCamera)
	{
		pSceneDatabase->GetCameras().clear();
	}

	if (!keepLight)
	{
		pSceneDatabase->GetLights().clear();
	}
}

// Translate different 3D model file formats to memory data.
void AssetBrowser::ImportModelFile(const char* pFilePath)
{
	engine::RenderContext* pCurrentRenderContext = GetRenderContext();
	engine::SceneWorld* pSceneWorld = GetImGuiContextInstance()->GetSceneWorld();

	cd::SceneDatabase* pSceneDatabase = pSceneWorld->GetSceneDatabase();
	uint32_t oldNodeCount = pSceneDatabase->GetNodeCount();
	uint32_t oldMeshCount = pSceneDatabase->GetMeshCount();

	// Step 1 : Convert model file to cd::SceneDatabase
	std::filesystem::path inputFilePath(pFilePath);
	std::filesystem::path inputFileExtension = inputFilePath.extension();
	if (0 == inputFileExtension.compare(".cdbin"))
	{
		cdtools::CDProducer cdProducer(pFilePath);
		cdtools::Processor processor(&cdProducer, nullptr, pSceneDatabase);
		processor.Run();
	}
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

		cdtools::Processor processor(&genericProducer, nullptr, pSceneDatabase);
		processor.SetDumpSceneDatabaseEnable(false);
		processor.SetFlattenSceneDatabaseEnable(true);
		processor.Run();
	}

	// Step 2 : Process generated cd::SceneDatabase
	ProcessSceneDatabase(pSceneDatabase, m_importOptions.ImportMesh, m_importOptions.ImportMaterial, m_importOptions.ImportTexture,
		m_importOptions.ImportCamera, m_importOptions.ImportLight);

	// Step 3 : Convert cd::SceneDatabase to entities and components
	{
		ECWorldConsumer ecConsumer(pSceneWorld, pCurrentRenderContext);
		ecConsumer.SetSceneDatabaseIDs(oldNodeCount, oldMeshCount);
		if (m_importOptions.AssetType == ImportAssetType::DDGIModel)
		{
			ecConsumer.ActivateDDGIService();
		}
		cdtools::Processor processor(nullptr, &ecConsumer, pSceneDatabase);
		processor.SetDumpSceneDatabaseEnable(true);
		processor.Run();
	}

	// Step 4 : Convert cd::SceneDatabase to cd asset files and save in disk
	{
		cdtools::CDConsumer cdConsumer(m_currentDirectory->FilePath.string().c_str());
		cdConsumer.SetExportMode(cdtools::ExportMode::XmlBinary);
		cdtools::Processor processor(nullptr, &cdConsumer, pSceneDatabase);
		processor.SetDumpSceneDatabaseEnable(false);
		processor.Run();
	}
}

void AssetBrowser::ExportAssetFile(const char* pFilePath)
{
	engine::SceneWorld* pSceneWorld = GetImGuiContextInstance()->GetSceneWorld();
	cd::SceneDatabase* pSceneDatabase = pSceneWorld->GetSceneDatabase();

	if (ExportAssetType::SceneDatabase == m_exportOptions.AssetType)
	{
		ProcessSceneDatabase(pSceneDatabase, m_exportOptions.ExportMesh, m_exportOptions.ExportMaterial, m_exportOptions.ExportTexture,
			m_exportOptions.ExportCamera, m_exportOptions.ExportLight);

		std::filesystem::path selectFilePath(pFilePath);
		std::filesystem::path outputFilePath = selectFilePath.replace_extension(".cdbin");
		cdtools::CDConsumer consumer(outputFilePath.string().c_str());
		cdtools::Processor processor(nullptr, &consumer, pSceneDatabase);
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
		ImGui::SetColumnWidth(0, ImGui::GetWindowContentRegionMax().x * 0.3f);
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
		m_importOptions.Active = true;
	}
	else
	{
		m_importOptions.AssetType = ImportAssetType::Unknown;
	}
	
	if (UpdateOptionDialog("Import Options", m_importOptions.Active, m_importOptions.ImportMesh, m_importOptions.ImportMaterial, m_importOptions.ImportTexture,
		m_importOptions.ImportCamera, m_importOptions.ImportLight))
	{
		ImportAssetFile(m_pImportFileBrowser->GetSelected().string().c_str());
	}
	if (!m_importOptions.Active)
	{
		m_pImportFileBrowser->ClearSelected();
		m_importOptions.Active = false;
	}

	m_pExportFileBrowser->Display();
	if (m_pExportFileBrowser->HasSelected())
	{
		m_exportOptions.Active = true;
	}

	if (UpdateOptionDialog("Export Options", m_exportOptions.Active, m_exportOptions.ExportMesh, m_exportOptions.ExportMaterial, m_exportOptions.ExportTexture,
		m_exportOptions.ExportCamera, m_exportOptions.ExportLight))
	{
		ExportAssetFile(m_pExportFileBrowser->GetSelected().string().c_str());
		m_pExportFileBrowser->ClearSelected();
	}
}

}