#include "ImGui/ImGuiBaseLayer.h"

#include <memory>

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
	DDGI,
	Unknown,
};

enum class ExportAssetType
{
	SceneDatabase,
	Unknown,
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

private:
	void UpdateAssetFolderTree();
	void UpdateAssetFileView();

private:
	ImportAssetType m_importingAssetType = ImportAssetType::Unknown;
	ExportAssetType m_exportingAssetType = ExportAssetType::Unknown;
	std::unique_ptr<ImGui::FileBrowser> m_pImportFileBrowser;
	std::unique_ptr<ImGui::FileBrowser> m_pExportFileBrowser;
	engine::Renderer* m_pSceneRenderer = nullptr;
};

}