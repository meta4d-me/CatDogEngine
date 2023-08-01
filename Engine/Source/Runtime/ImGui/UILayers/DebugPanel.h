#include "ImGui/ImGuiBaseLayer.h"

#include <memory>
namespace engine
{

class CameraController;

class DebugPanel : public engine::ImGuiBaseLayer
{
public:
	using ImGuiBaseLayer::ImGuiBaseLayer;
	virtual ~DebugPanel();

	virtual void Init() override;
	virtual void Update() override;

	void SetCameraController(std::shared_ptr<engine::CameraController> cameraController) { m_pCameraController = cameraController; }

	void ShowProfiler();

private:
	std::shared_ptr<engine::CameraController> m_pCameraController;
};

}