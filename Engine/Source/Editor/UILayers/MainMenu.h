#include "ImGui/ImGuiBaseLayer.h"

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
	void AboutMenu();
};

}