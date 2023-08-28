#include "ImGui/ImGuiBaseLayer.h"

#include "Display/CameraController.h"
namespace editor
{
class CameraController;
class SceneView;

class ImGuizmoView : public engine::ImGuiBaseLayer
{
public:
	using ImGuiBaseLayer::ImGuiBaseLayer;
	virtual ~ImGuizmoView();

	virtual void Init() override;
	virtual void Update() override;

	void SetSceneView(const SceneView* pSceneView) { m_pSceneView = pSceneView; }

	void SetCameraController(engine::CameraController* pCameraController) { m_pCameraController = pCameraController; }

private:
	const SceneView* m_pSceneView = nullptr;
	engine::CameraController* m_pCameraController = nullptr;
};

}