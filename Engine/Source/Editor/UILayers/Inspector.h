#include "ImGui/ImGuiBaseLayer.h"
#include "ImGui/ImGuiUtils.hpp"
#include "Scene/MaterialTextureType.h"

#include <filesystem>
#include <optional>

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

	void SetSelectMaterialTextureType(cd::MaterialTextureType textureType) { m_optSelectMaterialTextureType = textureType; }

	void SetIsOpenFileBrowser(bool flag) { m_isOpenFileBrowser = flag; }
	bool IsOpenFileBrowser() const { return m_isOpenFileBrowser; }

private:
	engine::Entity m_lastSelectedEntity = engine::INVALID_ENTITY;

	// Select file
	std::optional<cd::MaterialTextureType> m_optSelectMaterialTextureType;
	bool m_isOpenFileBrowser = false;
	std::unique_ptr<ImGui::FileBrowser> m_pSelectFileBrowser;
};

}