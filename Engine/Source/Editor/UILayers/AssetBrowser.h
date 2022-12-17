#include "ImGui/ImGuiBaseLayer.h"

namespace ImGui
{

class FileBrowser;

}

namespace engine
{

class SceneRenderer;

}

namespace editor
{

class AssetBrowser : public engine::ImGuiBaseLayer
{
public:
	using ImGuiBaseLayer::ImGuiBaseLayer;
	virtual ~AssetBrowser();

	virtual void Init() override;
	virtual void Update() override;

	void SetSceneRenderer(engine::SceneRenderer* pRenderer) { m_pSceneRenderer = pRenderer; }

private:
	void UpdateAssetFolderTree();
	void UpdateAssetFileView();

private:
	ImGui::FileBrowser* m_pImportFileBrowser;
	engine::SceneRenderer* m_pSceneRenderer = nullptr;
};

}