#pragma once

#include "ImGui/ImGuiBaseLayer.h"

#include <memory>

namespace ImGui
{

class FileBrowser;

}

namespace engine
{

class ViewportCameraController;

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

	void SetCameraController(engine::ViewportCameraController* pCameraController) { m_pCameraController = pCameraController; }

private:
	std::unique_ptr<ImGui::FileBrowser> m_pCreateProjectDialog;
	engine::ViewportCameraController* m_pCameraController = nullptr;
};

}