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

class EditorSceneWorld;

class AssetBrowser : public engine::ImGuiBaseLayer
{
public:
	using ImGuiBaseLayer::ImGuiBaseLayer;
	virtual ~AssetBrowser();

	virtual void Init() override;
	virtual void Update() override;

	void ImportAssetFile(const char* pFilePath);
	void SetSceneWorld(EditorSceneWorld* pWorld) { m_pEditorSceneWorld = pWorld; }
	void SetSceneRenderer(engine::Renderer* pRenderer) { m_pSceneRenderer = pRenderer; }

private:
	void UpdateAssetFolderTree();
	void UpdateAssetFileView();

private:
	ImGui::FileBrowser* m_pImportFileBrowser = nullptr;
	engine::Renderer* m_pSceneRenderer = nullptr;

	EditorSceneWorld* m_pEditorSceneWorld = nullptr;
};

}