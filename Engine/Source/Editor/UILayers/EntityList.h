#include "ImGui/ImGuiBaseLayer.h"

#include "ECWorld/Entity.h"

#include <imgui/imgui.h>
#include <memory>

namespace engine
{

class CameraController;
class SceneWorld;

}

namespace editor
{

class EntityList : public engine::ImGuiBaseLayer
{
public:
	using ImGuiBaseLayer::ImGuiBaseLayer;
	virtual ~EntityList();

	virtual void Init() override;
	virtual void Update() override;

	void AddEntity(engine::SceneWorld* pSceneWorld);
	void DrawEntity(engine::SceneWorld* pSceneWorld, engine::Entity entity);

	void SetCameraController(engine::CameraController* pCameraController) { m_pCameraController = pCameraController; }


private:
	ImGuiTextFilter m_entityFilter;
	engine::CameraController* m_pCameraController = nullptr;
};

}
