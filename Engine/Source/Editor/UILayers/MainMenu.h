#include "ImGui/ImGuiBaseLayer.h"

#include <memory>

namespace ImGui
{

class FileBrowser;

}

namespace engine
{

class CameraController;

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
	void ViewMenu();
	void WindowMenu();
	void BuildMenu();
	void AboutMenu();

	void SetCameraController(engine::CameraController* pCameraController) { m_pCameraController = pCameraController; }

private:
	std::unique_ptr<ImGui::FileBrowser> m_pCreatProjectDialog;
	engine::CameraController* m_pCameraController = nullptr;
};

}