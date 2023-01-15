#include "ImGui/ImGuiBaseLayer.h"

namespace engine
{
	class Window;
}

namespace editor
{

class MainMenu : public engine::ImGuiBaseLayer
{
public:
	using ImGuiBaseLayer::ImGuiBaseLayer;

	explicit MainMenu(const char* pName, engine::Window* mainWindow);
	virtual ~MainMenu();

	virtual void Init() override;
	virtual void Update() override;

	void FileMenu();
	void EditMenu();
	void WindowMenu();
	void AboutMenu();

private:
	engine::Window* p_MainWindow;

};

}