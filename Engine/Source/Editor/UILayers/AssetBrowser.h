#include "ImGui/ImGuiBaseLayer.h"

#include <filesystem>
#include <memory>
#include <unordered_map>

namespace ImGui
{

class FileBrowser;

}

namespace cd
{

class SceneDatabase;

}

namespace engine
{

class Renderer;

}

namespace editor
{

enum class ImportAssetType
{
	CubeMap,
	Model,
	Terrain,
	Shader,
	Unknown,
};

enum class ExportAssetType
{
	SceneDatabase,
	Unknown,
};

class DirectoryInformation
{
public:
	DirectoryInformation(const std::filesystem::path& fname, bool isF)
	{
		FilePath = fname;
		IsFile = isF;
	}

	std::shared_ptr<DirectoryInformation> Parent;
	std::vector<std::shared_ptr<DirectoryInformation>> Children;

	std::filesystem::path FilePath;
	bool IsFile;
};

class AssetBrowser : public engine::ImGuiBaseLayer
{
public:
	using ImGuiBaseLayer::ImGuiBaseLayer;
	virtual ~AssetBrowser();

	virtual void Init() override;
	virtual void Update() override;

	void ImportAssetFile(const char* pFilePath);
	void ImportModelFile(const char* pFilePath);
	void ExportAssetFile(const char* pFilePath);
	void SetSceneRenderer(engine::Renderer* pRenderer) { m_pSceneRenderer = pRenderer; }
	void DrawFolder(const std::shared_ptr<DirectoryInformation>& dirInfo, bool defaultOpen = false);
	void ChangeDirectory(std::shared_ptr<DirectoryInformation>& directory);

	std::string ProcessDirectory(const std::filesystem::path& directoryPath, const std::shared_ptr<DirectoryInformation>& parent);
	std::shared_ptr<DirectoryInformation> CreateDirectoryInfoSharedPtr(const std::filesystem::path& directoryPath, bool isDirectory);
	std::string StripExtras(const std::string& filename);
	bool IsHiddenFile(const std::string& path);
	bool IsTextureFile(const std::string& extension);
	bool RenderFile(int dirIndex, bool folder, int shownIndex, bool gridView);

private:
	void UpdateAssetFolderTree();
	void UpdateAssetFileView();

private:
	ImportAssetType m_importingAssetType = ImportAssetType::Unknown;
	ExportAssetType m_exportingAssetType = ExportAssetType::Unknown;
	std::unique_ptr<ImGui::FileBrowser> m_pImportFileBrowser;
	std::unique_ptr<ImGui::FileBrowser> m_pExportFileBrowser;
	engine::Renderer* m_pSceneRenderer = nullptr;

	bool m_UpdateNavigationPath = true;
	bool m_ShowHiddenFiles;
	bool m_IsInListView;
	int m_GridItemPerRow;
	float m_GridSize = 50.0f;
	float m_scale = 40.0f;

	std::string m_BasePath;
	std::filesystem::path m_AssetPath;

	std::shared_ptr<DirectoryInformation> m_CurrentDir;
	std::shared_ptr<DirectoryInformation> m_BaseProjectDir;
	std::shared_ptr<DirectoryInformation> m_NextDirectory;
	std::shared_ptr<DirectoryInformation> m_PreviousDirectory;
	std::unordered_map<std::string, std::shared_ptr<DirectoryInformation>> m_Directories;
	std::vector<std::shared_ptr<DirectoryInformation>> m_BreadCrumbData;
};

}