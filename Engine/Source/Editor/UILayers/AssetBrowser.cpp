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
#include "Producers/EffekseerProducer/EffekseerProducer.h"
#include "Rendering/WorldRenderer.h"
#include "Rendering/RenderContext.h"
#include "Resources/ResourceBuilder.h"
#include "Resources/ResourceLoader.h"

#ifdef ENABLE_GENERIC_PRODUCER
#include "Producers/GenericProducer/GenericProducer.h"
#endif

#include <json/json.hpp>

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

bool IsLightInputFile(const char* pFileExtension)
{
	constexpr const char* pFileExtensions[] = { ".json" };
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

bool IsParticleInputFile(const char* pFileExtension)
{
	constexpr const char* pFileExtensions[] = { ".efkefc" };
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

cd::Vec3f GetVec3fFormString(const std::string& str)
{
	std::string digits;
	size_t start = str.find("(");
	size_t end = str.find(")");
	if (start != std::string::npos && end != std::string::npos)
	{
		digits = str.substr(start + 1, end - start - 1);
	}
	std::istringstream iss(digits);
	float num1, num2, num3;
	char comma;
	iss >> num1 >> comma >> num2 >> comma >> num3;
	return cd::Vec3f(num1, num2, num3);
}

cd::Vec2f GetVec2fFormString(const std::string& str)
{
	std::string digits;
	size_t start = str.find("(");
	size_t end = str.find(")");
	if (start != std::string::npos && end != std::string::npos)
	{
		digits = str.substr(start + 1, end - start - 1);
	}
	std::istringstream iss(digits);
	float num1, num2;
	char comma;
	iss >> num1 >> comma >> num2;
	return cd::Vec2f(num1, num2);
}

float GetFloatFormString(const std::string& str)
{
	float num;
	std::istringstream iss(str);
	iss >> num; 
	return num;
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
				 std::filesystem::path texviewPath = resourcesPath / "Textures/" / "textures" / (nameNoEx + ".dds");
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
					 ImGui::Image(reinterpret_cast<ImTextureID>(TextureHandle.idx), img_size);
				 }

			 }
			/* if (0 == strcmp(".dds", extension.c_str()))
			 {
				 ImGui::Button(reinterpret_cast<const char*>(ICON_MDI_DELTA), ImVec2(m_gridSize, m_gridSize));
			 }*/
	
						
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
	m_currentDirectory		= directory;
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
			m_importOptions.AssetType = IOAssetType::CubeMap;
			m_pImportFileBrowser->SetTitle("ImportAssets - Cubemap");
			//m_pImportFileBrowser->SetTypeFilters({ ".dds", "*.exr", "*.hdr", "*.ktx", ".tga" });
			m_pImportFileBrowser->Open();

			CD_INFO("Import asset type: {}", nameof::nameof_enum(m_importOptions.AssetType));
		}

		else if (ImGui::Selectable("Shader"))
		{
			m_importOptions.AssetType = IOAssetType::Shader;
			m_pImportFileBrowser->SetTitle("ImportAssets - Shader");
			//m_pImportFileBrowser->SetTypeFilters({ ".sc" }); // ".hlsl"
			m_pImportFileBrowser->Open();

			CD_INFO("Import asset type: {}", nameof::nameof_enum(m_importOptions.AssetType));
		}
		else if (ImGui::Selectable("Model"))
		{
			m_importOptions.AssetType = IOAssetType::Model;
			m_pImportFileBrowser->SetTitle("ImportAssets - Model");
			//m_pImportFileBrowser->SetTypeFilters({ ".fbx", ".gltf" }); // ".obj", ".dae", ".ogex"
			m_pImportFileBrowser->Open();

			CD_INFO("Import asset type: {}", nameof::nameof_enum(m_importOptions.AssetType));
		}
		else if (ImGui::Selectable("Celluloid Model"))
		{
			m_importOptions.AssetType = IOAssetType::Model;
			m_pImportFileBrowser->SetTitle("ImportAssets - Celluloid Model");
			//m_pImportFileBrowser->SetTypeFilters({ ".fbx", ".gltf" }); // ".obj", ".dae", ".ogex"
			m_pImportFileBrowser->Open();

			CD_INFO("Import asset type: {}", nameof::nameof_enum(m_importOptions.AssetType));
		}

#ifdef ENABLE_DDGI
		else if (ImGui::Selectable("DDGI Model"))
		{
			m_importOptions.AssetType = IOAssetType::DDGIModel;
			m_pImportFileBrowser->SetTitle("ImportAssets - DDGI Model");
			m_pImportFileBrowser->Open();

			CD_INFO("Import asset type: {}", GetIOAssetTypeName(m_importOptions.AssetType));
		}
		else if (ImGui::Selectable("Light from json"))
		{
			m_importOptions.AssetType = IOAssetType::Light;
			m_pImportFileBrowser->SetTitle("ImportAssets - Light");
			m_pImportFileBrowser->Open();
			CD_INFO("Import asset type: {}", GetIOAssetTypeName(m_importOptions.AssetType));
		}
#endif

		ImGui::EndPopup();
	}

	if (ImGui::BeginPopup("ExportAssets"))
	{
		if (ImGui::Selectable("SceneDatabase"))
		{
			m_exportOptions.AssetType = IOAssetType::SceneDatabase;

			// TODO : The file browser seems impossible to export file.
			// Here I will use the select file as file name of exported file name.
			m_pExportFileBrowser->SetTitle("ExportAssets - SceneDatabase");
			m_pExportFileBrowser->Open();

			CD_INFO("Export asset type: {}", nameof::nameof_enum(m_exportOptions.AssetType));
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

bool AssetBrowser::UpdateOptionDialog(const char* pTitle, bool& active, bool& importMesh, bool& importMaterial, bool& importTexture, bool& importAnimation, bool& importCamera, bool& importLight)
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
			ImGuiUtils::ImGuiBoolProperty("Mesh", importMesh);
		}

		ImGui::Separator();
		ImGui::PopStyleVar();

		bool isMaterialOpen = ImGui::CollapsingHeader("Material Options", ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_DefaultOpen);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
		ImGui::Separator();
		if (isMaterialOpen)
		{
			ImGuiUtils::ImGuiBoolProperty("Material", importMaterial);
			ImGuiUtils::ImGuiBoolProperty("Texture", importTexture);
		}
		ImGui::Separator();
		ImGui::PopStyleVar();

		bool isOtherOpen = ImGui::CollapsingHeader("Other", ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_DefaultOpen);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
		ImGui::Separator();
		if (isOtherOpen)
		{
			ImGuiUtils::ImGuiBoolProperty("Animation", importAnimation);
			ImGuiUtils::ImGuiBoolProperty("Camera", importCamera);
			ImGuiUtils::ImGuiBoolProperty("Light", importLight);
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
			m_importOptions.AssetType = IOAssetType::Unknown;
			m_pImportFileBrowser->ClearSelected();
		}
		ImGui::EndPopup();
	}
	return finish;
}

void AssetBrowser::ImportAssetFile(const char* pFilePath)
{
	CD_INFO("Importing asset file : {0}", pFilePath);
	// Unknown is used by outside window event such as DragAndDrop a file.
	if (IOAssetType::Unknown == m_importOptions.AssetType)
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
			m_importOptions.AssetType = IOAssetType::CubeMap;
		}
		else if (IsShaderInputFile(pFileExtension.c_str()))
		{
			m_importOptions.AssetType = IOAssetType::Shader;
		}
		else if (IsModelInputFile(pFileExtension.c_str()))
		{
			m_importOptions.AssetType = IOAssetType::Model;
		}
		else if (IsLightInputFile(pFileExtension.c_str()))
		{
			m_importOptions.AssetType = IOAssetType::Light;
		}
		else if (IsParticleInputFile(pFileExtension.c_str()))
		{
			m_importOptions.AssetType = IOAssetType::Particle;
		}
		else
		{
			// Still unknown, exit.
			CD_WARN("Unable to deduce import asset type by file extension : {0}", pFileExtension.c_str());
			return;
		}
	}

	if (IOAssetType::Model == m_importOptions.AssetType || IOAssetType::DDGIModel == m_importOptions.AssetType)
	{
		ImportModelFile(pFilePath);
	}
	else if (IOAssetType::CubeMap == m_importOptions.AssetType)
	{
		engine::SceneWorld* pSceneWorld = GetImGuiContextInstance()->GetSceneWorld();
		engine::SkyComponent* pSkyComponent = pSceneWorld->GetSkyComponent(pSceneWorld->GetSkyEntity());

		if (engine::SkyType::SkyBox == pSkyComponent->GetSkyType())
		{
			std::string relativePath = (std::filesystem::path("Textures") /
				std::filesystem::path(pFilePath).stem()).generic_string();

			std::filesystem::path absolutePath = CDPROJECT_RESOURCES_ROOT_PATH;
			absolutePath /= relativePath;

			CD_INFO("Compile skybox textures to {0}.", absolutePath);

			std::string irrdianceOutput = absolutePath.generic_string() + "_irr.dds";
			ResourceBuilder::Get().AddIrradianceCubeMapBuildTask(pFilePath, irrdianceOutput.c_str());
			ResourceBuilder::Get().Update();

			std::string radianceOutput = absolutePath.generic_string() + "_rad.dds";
			ResourceBuilder::Get().AddRadianceCubeMapBuildTask(pFilePath, radianceOutput.c_str());
			ResourceBuilder::Get().Update();

			pSkyComponent->SetIrradianceTexturePath(relativePath + "_irr.dds");
			pSkyComponent->SetRadianceTexturePath(relativePath + "_rad.dds");
		}
	}
	else if (IOAssetType::Shader == m_importOptions.AssetType)
	{
		std::filesystem::path inputFilePath(pFilePath);
		std::string inputFileName = inputFilePath.stem().generic_string();

		engine::ShaderType shaderType;
		if (inputFileName.find("vs_") != std::string::npos)
		{
			shaderType = engine::ShaderType::Vertex;
		}
		else if (inputFileName.find("fs_") != std::string::npos)
		{
			shaderType = engine::ShaderType::Fragment;
		}
		else if (inputFileName.find("cs_") != std::string::npos)
		{
			shaderType = engine::ShaderType::Compute;
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
	else if (IOAssetType::Light == m_importOptions.AssetType)
	{
		ImportJson(pFilePath);
	}
	else if (IOAssetType::Particle == m_importOptions.AssetType)
	{
		ImportParticleEffect(pFilePath);
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
			for (auto& materialID : mesh.GetMaterialIDs())
			{
				materialID.Set(cd::MaterialID::InvalidID);
			}
		}
	}

	if (!keepTexture)
	{
		pSceneDatabase->GetTextures().clear();
		for (auto& material : pSceneDatabase->GetMaterials())
		{
			for (int textureTypeIndex = 0; textureTypeIndex < nameof::enum_count<cd::MaterialTextureType>(); ++textureTypeIndex)
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
		cd::SceneDatabase newSceneDatabase;
		cdtools::Processor processor(&cdProducer, nullptr, &newSceneDatabase);
		processor.Run();
		pSceneDatabase->Merge(cd::MoveTemp(newSceneDatabase));
	}
	else
	{
#ifdef ENABLE_GENERIC_PRODUCER
		cdtools::GenericProducer genericProducer(pFilePath);
		genericProducer.EnableOption(cdtools::GenericProducerOptions::GenerateBoundingBox);
		genericProducer.EnableOption(cdtools::GenericProducerOptions::CleanUnusedObjects);
		genericProducer.EnableOption(cdtools::GenericProducerOptions::GenerateTangentSpace);
		genericProducer.EnableOption(cdtools::GenericProducerOptions::TriangulateModel);
		if (!m_importOptions.ImportAnimation)
		{
			genericProducer.EnableOption(cdtools::GenericProducerOptions::FlattenTransformHierarchy);
		}

		cd::SceneDatabase newSceneDatabase;
		cdtools::Processor processor(&genericProducer, nullptr, &newSceneDatabase);
		processor.EnableOption(cdtools::ProcessorOptions::Dump);
		processor.Run();
		pSceneDatabase->Merge(cd::MoveTemp(newSceneDatabase));
#else
		assert("Unable to import this file format.");
#endif
	}

	// Step 2 : Process generated cd::SceneDatabase
	ProcessSceneDatabase(pSceneDatabase, m_importOptions.ImportMesh, m_importOptions.ImportMaterial, m_importOptions.ImportTexture,
		m_importOptions.ImportCamera, m_importOptions.ImportLight);

	// Step 3 : Convert cd::SceneDatabase to entities and components
	{
		ECWorldConsumer ecConsumer(pSceneWorld, pCurrentRenderContext);
		ecConsumer.SetDefaultMaterialType(pSceneWorld->GetCelluloidMaterialType());
		ecConsumer.SetSceneDatabaseIDs(oldNodeCount, oldMeshCount);
#ifdef ENABLE_DDGI
		if (m_importOptions.AssetType == IOAssetType::DDGIModel)
		{
			ecConsumer.SetDefaultMaterialType(pSceneWorld->GetDDGIMaterialType());
		}
#endif
		cdtools::Processor processor(nullptr, &ecConsumer, pSceneDatabase);
		processor.EnableOption(cdtools::ProcessorOptions::Dump);
		processor.Run();
	}

	// Step 4 : Convert cd::SceneDatabase to cd asset files and save in disk
	{
		cdtools::CDConsumer cdConsumer(m_currentDirectory->FilePath.string().c_str());
		cdConsumer.SetExportMode(cdtools::ExportMode::XmlBinary);
		cdtools::Processor processor(nullptr, &cdConsumer, pSceneDatabase);
		processor.DisableOption(cdtools::ProcessorOptions::Dump);
		processor.Run();
	}
}

void AssetBrowser::ImportJson(const char* pFilePath)
{
	engine::SceneWorld* pSceneWorld = GetSceneWorld();
	engine::World* pWorld = pSceneWorld->GetWorld();
	cd::SceneDatabase* pSceneDatabase = pSceneWorld->GetSceneDatabase();

	auto AddNamedEntity = [&pWorld](std::string defaultName) -> engine::Entity
	{
		engine::Entity entity = pWorld->CreateEntity();
		auto& nameComponent = pWorld->CreateComponent<engine::NameComponent>(entity);
		nameComponent.SetName(defaultName + std::to_string(entity));

		return entity;
	};

	auto CreateLightComponents = [&pWorld](engine::Entity entity, cd::LightType lightType, float intensity, cd::Vec3f color) -> engine::LightComponent&
	{
		auto& lightComponent = pWorld->CreateComponent<engine::LightComponent>(entity);
		lightComponent.SetType(lightType);
		lightComponent.SetIntensity(intensity);
		lightComponent.SetColor(color);

		auto& transformComponent = pWorld->CreateComponent<engine::TransformComponent>(entity);
		transformComponent.SetTransform(cd::Transform::Identity());
		transformComponent.Build();

		return lightComponent;
	};

	std::ifstream file(pFilePath);
	if (file.is_open())
	{
		std::map<std::string, engine::MaterialComponent*> mapMaterialNameToMaterialData;

		for (engine::Entity matreialEntity : pSceneWorld->GetMaterialEntities())
		{
			engine::MaterialComponent* pMatreialComponent = pSceneWorld->GetMaterialComponent(matreialEntity);

			mapMaterialNameToMaterialData[pMatreialComponent->GetName()] = pMatreialComponent;
		}

		std::stringstream buffer;
		buffer << file.rdbuf();
		std::string jsonString = buffer.str();
		file.close();
		nlohmann::json jsonData = nlohmann::json::parse(jsonString);
		nlohmann::json lightsArray = jsonData["lights"];
		nlohmann::json materialArray = jsonData["mats"];
		for (auto& light : lightsArray)
		{
			std::string name = light["name"];
			std::string lightType = light["type"];
			std::string color = light["color"];
			std::string intensity = light["intensity"];
			std::string position = light["worldpos"];
			std::string lightDir = light["worlddir"];
			std::string range = light["range"];
			std::string innerangle = light["innerSpotAngle"];
			std::string outerangle = light["spotAngle"];
			engine::Entity entity = AddNamedEntity(light["name"]);
			if (0 == std::strcmp(lightType.c_str(),"Point"))
			{
				auto& lightComponent = CreateLightComponents(entity, cd::LightType::Point, GetFloatFormString(intensity), GetVec3fFormString(color) / 255.0f);
				lightComponent.SetIntensity(90.0f);
				lightComponent.SetPosition(GetVec3fFormString(position) + cd::Vec3f(-1.5f, 0.0f, 0.0f));
				lightComponent.SetRange(GetFloatFormString(range));
			}
			else if (0 == std::strcmp(lightType.c_str(), "Directional"))
			{
				auto& lightComponent = CreateLightComponents(entity, cd::LightType::Directional, GetFloatFormString(intensity), GetVec3fFormString(color) / 255.0f);
				lightComponent.SetIntensity(4.0f);
				lightComponent.SetDirection(GetVec3fFormString(lightDir));
			}
			else if (0 == std::strcmp(lightType.c_str(), "Spot"))
			{
				auto& lightComponent = CreateLightComponents(entity, cd::LightType::Spot, GetFloatFormString(intensity), GetVec3fFormString(color) / 255.0f);
				lightComponent.SetIntensity(90.0f);
				lightComponent.SetPosition(GetVec3fFormString(position) + cd::Vec3f(-1.5f, 0.0f, 0.0f));
				lightComponent.SetDirection(GetVec3fFormString(lightDir));
				lightComponent.SetRange(GetFloatFormString(range));
				lightComponent.SetInnerAndOuter(GetFloatFormString(innerangle), GetFloatFormString(outerangle));
			}
		}

		for (auto& material : materialArray)
		{
			std::string name = material["name"];
			std::string color = material["color@_Color"];
			std::string UVOffset = material["tex_tiling@_MainTex"];
			std::string UVScale = material["tex_scale@_MainTex"];
			float roughness = material["float@_Glossiness"];
			float metallic = material["float@_Metallic"];
			if (engine::MaterialComponent* pMaterialComponent = mapMaterialNameToMaterialData[name])
			{
				pMaterialComponent->SetFactor(cd::MaterialPropertyGroup::BaseColor, GetVec3fFormString(color) / 255.0f);
				pMaterialComponent->SetFactor(cd::MaterialPropertyGroup::Metallic, metallic);
				pMaterialComponent->SetFactor(cd::MaterialPropertyGroup::Roughness, roughness);
				if (auto pPropertyGroup = pMaterialComponent->GetPropertyGroup(cd::MaterialPropertyGroup::BaseColor); pPropertyGroup)
				{
					pPropertyGroup->textureInfo.SetUVOffset(GetVec2fFormString(UVOffset));
					pPropertyGroup->textureInfo.SetUVScale(GetVec2fFormString(UVScale));
				}
			}
			else
			{
				CD_ERROR("Not find Material");
			}
		}
	}
	else
	{
		CD_INFO("Open Joson file failed");
	}
}

void AssetBrowser::ImportParticleEffect(const char* pFilePath)
{
	////engine::RenderContext* pCurrentRenderContext = GetRenderContext();
	//engine::SceneWorld* pSceneWorld = GetImGuiContextInstance()->GetSceneWorld();

	//cd::SceneDatabase* pSceneDatabase = pSceneWorld->GetSceneDatabase();
	////uint32_t oldNodeCount = pSceneDatabase->GetNodeCount();
	////uint32_t oldMeshCount = pSceneDatabase->GetMeshCount();
	//uint32_t oldParticleEmitterCount = pSceneDatabase->GetParticleEmitterCount();

	//// Step 1 : Convert model file to cd::SceneDatabase
	//std::filesystem::path inputFilePath(pFilePath);
	//std::filesystem::path inputFileExtension = inputFilePath.extension();
	//if (0 == inputFileExtension.compare(".efkefc"))
	//{
	///*	cdtools::CDProducer cdProducer(pFilePath);
	//	cd::SceneDatabase newSceneDatabase;
	//	cdtools::Processor processor(&cdProducer, nullptr, &newSceneDatabase);
	//	proce qssor.Run();
	//	pSceneDatabase->Merge(cd::MoveTemp(newSceneDatabase));*/
	//	std::string filePath = pFilePath;
	//	int size = MultiByteToWideChar(CP_UTF8, 0, filePath.c_str(), -1, nullptr, 0);
	//	std::wstring wstr(size, 0);
	//	MultiByteToWideChar(CP_UTF8, 0, filePath.c_str(), -1, &wstr[0], size);
	//	std::wstring wFilePath = wstr;
	//	const char16_t* u16_cstr = reinterpret_cast<const char16_t*>(wFilePath.c_str());
	//	cdtools::EffekseerProducer efkProducer(u16_cstr);
	//}
}

void AssetBrowser::ExportAssetFile(const char* pFilePath)
{
	engine::SceneWorld* pSceneWorld = GetImGuiContextInstance()->GetSceneWorld();
	cd::SceneDatabase* pSceneDatabase = pSceneWorld->GetSceneDatabase();

	if (IOAssetType::SceneDatabase == m_exportOptions.AssetType)
	{
		// Clean cameras and lights. Then convert current latest camera/light component data to SceneDatabase.
		ProcessSceneDatabase(pSceneDatabase, m_exportOptions.ExportMesh, m_exportOptions.ExportMaterial, m_exportOptions.ExportTexture,
			false/*keepCamera*/, false/*keepLight*/);

		for (auto entity : pSceneWorld->GetCameraEntities())
		{
			pSceneWorld->AddCameraToSceneDatabase(entity);
		}

		//pSceneDatabase->SetLightCount(static_cast<uint32_t>(pSceneWorld->GetLightEntities().size()));
		for (auto entity : pSceneWorld->GetLightEntities())
		{
			pSceneWorld->AddLightToSceneDatabase(entity);
		}

		for (auto entity : pSceneWorld->GetMaterialEntities())
		{
			pSceneWorld->AddMaterialToSceneDatabase(entity);
		}

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
	if (UpdateOptionDialog("Import Options", m_importOptions.Active, m_importOptions.ImportMesh, m_importOptions.ImportMaterial, m_importOptions.ImportTexture,
		m_importOptions.ImportAnimation, m_importOptions.ImportCamera, m_importOptions.ImportLight))
	{
		ImportAssetFile(m_pImportFileBrowser->GetSelected().string().c_str());
		m_pImportFileBrowser->ClearSelected();
	}

	m_pExportFileBrowser->Display();
	if (m_pExportFileBrowser->HasSelected())
	{
		m_exportOptions.Active = true;
	}

	if (UpdateOptionDialog("Export Options", m_exportOptions.Active, m_exportOptions.ExportMesh, m_exportOptions.ExportMaterial, m_exportOptions.ExportTexture,
		m_importOptions.ImportAnimation, m_exportOptions.ExportCamera, m_exportOptions.ExportLight))
	{
		ExportAssetFile(m_pExportFileBrowser->GetSelected().string().c_str());
		m_pExportFileBrowser->ClearSelected();
	}
}

}
