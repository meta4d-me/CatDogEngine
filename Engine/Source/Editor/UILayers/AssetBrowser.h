#include "ImGui/ImGuiBaseLayer.h"

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
	Shader,
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
	void SetSceneRenderer(engine::Renderer* pRenderer) { m_pSceneRenderer = pRenderer; }

private:
	void UpdateAssetFolderTree();
	void UpdateAssetFileView();

private:
	ImGui::FileBrowser* m_pImportFileBrowser = nullptr;
	engine::Renderer* m_pSceneRenderer = nullptr;
	ImportAssetType m_importingAssetType = ImportAssetType::Unknown;
};

}