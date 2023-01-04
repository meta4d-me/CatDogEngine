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

class EditorWorld;

class AssetBrowser : public engine::ImGuiBaseLayer
{
public:
	using ImGuiBaseLayer::ImGuiBaseLayer;
	virtual ~AssetBrowser();

	virtual void Init() override;
	virtual void Update() override;

	void ImportAssetFile(const char* pFilePath);
	void SetWorld(EditorWorld* pWorld) { m_pEditorWorld = pWorld; }
	void SetSceneDatabase(cd::SceneDatabase* pSceneDatabase) { m_pSceneDatabase = pSceneDatabase; }
	void SetSceneRenderer(engine::Renderer* pRenderer) { m_pSceneRenderer = pRenderer; }

private:
	void UpdateAssetFolderTree();
	void UpdateAssetFileView();

private:
	ImGui::FileBrowser* m_pImportFileBrowser = nullptr;
	engine::Renderer* m_pSceneRenderer = nullptr;

	EditorWorld* m_pEditorWorld = nullptr;
	cd::SceneDatabase* m_pSceneDatabase = nullptr;
};

}