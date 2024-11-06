#include "Base/Platform.h"
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

enum class IOAssetType
{
	CubeMap,
	DDGIModel,
	Model,
	Shader,
	SceneDatabase,
	Terrain,
	Light,
	Particle,
	Unknown,
};

struct AssetImportOptions
{
	IOAssetType AssetType = IOAssetType::Unknown;
	bool Active = false;
	bool ImportCamera = false;
	bool ImportLight = false;
	bool ImportMaterial = true;
	bool ImportMesh = true;
	bool ImportTexture = true;
	bool ImportAnimation = true;
};

struct AssetExportOptions
{
	IOAssetType AssetType = IOAssetType::Unknown;
	bool Active = false;
	bool ExportCamera = true;
	bool ExportLight = true;
	bool ExportMaterial = true;
	bool ExportMesh = true;
	bool ExportTexture = true;
};

struct DirectoryInformation
{
	DirectoryInformation(const std::filesystem::path& fileName, bool isFile)
	{
		FilePath = fileName;
		IsFile = isFile;
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

	void SetSceneRenderer(engine::Renderer* pRenderer) { m_pSceneRenderer = pRenderer; }
	void ImportAssetFile(const char* pFilePath);
	void ExportAssetFile(const char* pFilePath);

private:
	void ProcessSceneDatabase(cd::SceneDatabase* pSceneDatabase, bool keepMesh, bool keepMaterial, bool keepTexture, bool keepCamera, bool keepLight);
	void ImportModelFile(const char* pFilePath);
	void ImportParticleEffect(const char* pFilePath);
	void ImportJson(const char* pFilePath);
	void DrawFolder(const std::shared_ptr<DirectoryInformation>& dirInfo, bool defaultOpen = false);
	void ChangeDirectory(std::shared_ptr<DirectoryInformation>& directory);
	
	std::string ProcessDirectory(const std::filesystem::path& directoryPath, const std::shared_ptr<DirectoryInformation>& parent);
	std::shared_ptr<DirectoryInformation> CreateDirectoryInfoSharedPtr(const std::filesystem::path& directoryPath, bool isDirectory);
	std::string StripExtras(const std::string& filename);
	bool IsHiddenFile(const std::string& path);
	bool IsTextureFile(const std::string& extension);
	bool RenderFile(int dirIndex, bool folder, int shownIndex, bool gridView);

	void UpdateAssetFolderTree();
	void UpdateAssetFileView();
	bool UpdateOptionDialog(const char* pTitle, bool& active, bool& importMesh, bool& importMaterial, bool& importTexture, bool& importAnimation, bool& importCamera, bool& importLight);

private:
	AssetImportOptions m_importOptions;
	AssetExportOptions m_exportOptions;
	std::unique_ptr<ImGui::FileBrowser> m_pImportFileBrowser;
	std::unique_ptr<ImGui::FileBrowser> m_pExportFileBrowser;

	engine::Renderer* m_pSceneRenderer = nullptr;

	bool m_updateNavigationPath = true;
	bool m_showHiddenFiles;
	bool m_isInListView;
	int m_gridItemPerRow;
	float m_gridSize = 40.0f;

	std::string m_basePath;
	std::shared_ptr<DirectoryInformation> m_currentDirectory;
	std::shared_ptr<DirectoryInformation> m_baseProjectDirectory;
	std::shared_ptr<DirectoryInformation> m_nextDirectory;
	std::shared_ptr<DirectoryInformation> m_previousDirectory;
	std::unordered_map<std::string, std::shared_ptr<DirectoryInformation>> m_directories;
	std::vector<std::shared_ptr<DirectoryInformation>> m_breadCrumbData;
};

}