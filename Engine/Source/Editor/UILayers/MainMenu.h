#pragma once

#include "ImGui/ImGuiBaseLayer.h"

#include <memory>

namespace ImGui
{

class FileBrowser;

}

namespace engine
{

class EditorCameraController;

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

	void SetCameraController(engine::EditorCameraController* pCameraController) { m_pCameraController = pCameraController; }

private:
	std::unique_ptr<ImGui::FileBrowser> m_pCreateProjectDialog;
	engine::EditorCameraController* m_pCameraController = nullptr;
};

}