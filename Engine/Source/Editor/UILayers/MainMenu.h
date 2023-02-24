#include "ImGui/ImGuiBaseLayer.h"

namespace ImGui
{
	class FileBrowser;
}
namespace editor
{

class MainMenu : public engine::ImGuiBaseLayer
{
public:
	using ImGuiBaseLayer::ImGuiBaseLayer;
	virtual ~MainMenu();

	virtual void Init() override;
	virtual void Update() override;

	void FileMenu();
	void EditMenu();
	void WindowMenu();
	void BuildMenu();
	void AboutMenu();
private:
	ImGui::FileBrowser* m_pCreatProject = nullptr;
};

}