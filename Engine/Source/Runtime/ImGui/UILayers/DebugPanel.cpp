#include "DebugPanel.h"
#include "Display/CameraController.h"

#include <imgui/imgui.h>

namespace engine
{

DebugPanel::~DebugPanel()
{

}

void DebugPanel::Init()
{

}

void DebugPanel::Update()
{
	ImGui::Begin(GetName(), &m_isEnable);
	if (ImGui::Selectable("Front"))
	{
		if (m_pCameraController)
			m_pCameraController->MoveToPosition(cd::Vec3f(-15.40f, 1.75f, 0.16f), cd::Vec3f(49.64f, 88.32f, 48.37f));
	}
	if (ImGui::Selectable("Left"))
	{
		if (m_pCameraController)
			m_pCameraController->MoveToPosition(cd::Vec3f(-2.40, 1.75f, -1.95f), cd::Vec3f(173.14f, 50.70f, 165.59f));
	}
	if (ImGui::Selectable("Right"))
	{
		if (m_pCameraController)
			m_pCameraController->MoveToPosition(cd::Vec3f(-4.71, 0.75f, 2.37f), cd::Vec3f(2.36f, 36.19f, 0.77f));
	}
	if (ImGui::Selectable("Behind"))
	{
		if (m_pCameraController)
			m_pCameraController->MoveToPosition(cd::Vec3f(9.18, 0.75f, -7.05f), cd::Vec3f(-25.74f, 33.71f, -7.92f));
	}
	ImGui::End();
}

}