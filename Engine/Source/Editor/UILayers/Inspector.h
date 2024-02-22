#include "ImGui/ImGuiBaseLayer.h"
#include "ImGui/ImGuiUtils.hpp"

#include <filesystem>

namespace ImGui
{

class FileBrowser;

}

namespace editor
{

class Inspector : public engine::ImGuiBaseLayer
{
public:
	using ImGuiBaseLayer::ImGuiBaseLayer;
	virtual ~Inspector();

	virtual void Init() override;
	virtual void Update() override;

	void SelectTexture();

	void SetIsOpenBrowser(bool isOpenBrowser) { m_isOpenBrowser = isOpenBrowser; }
	bool& GetIsOpenBrowser() { return m_isOpenBrowser; }
	bool GetIsOpenBrowser() const { return m_isOpenBrowser; }

private:
	bool m_isOpenBrowser = false;
	std::filesystem::path m_texturePath;
	engine::Entity m_lastSelectedEntity = engine::INVALID_ENTITY;
	std::unique_ptr<ImGui::FileBrowser> m_pImportFileBrowser;
};

}