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

	void SetIsOpenBrowser(bool isOpenBrowser) { m_isOpenBrowser = isOpenBrowser; }
	bool IsOpenBrowser() const { return m_isOpenBrowser; }

private:
	engine::Entity m_lastSelectedEntity = engine::INVALID_ENTITY;

	// Select file
	bool m_isOpenBrowser = false;
	std::filesystem::path m_lastSelectedFilePath;
	std::unique_ptr<ImGui::FileBrowser> m_pSelectFileBrowser;
};

}