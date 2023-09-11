#include "ImGui/ImGuiBaseLayer.h"

#include "Scene/Bone.h"
#include "Scene/SceneDatabase.h"

namespace engine
{

class CameraController;

}

namespace editor
{

class SkeletonView : public engine::ImGuiBaseLayer
{
public:
	using ImGuiBaseLayer::ImGuiBaseLayer;
	virtual ~SkeletonView();

	virtual void Init() override;
	virtual void Update() override;

	void DrawBone(engine::SceneWorld* pSceneWorld, const cd::Bone& Bone);
	void DrawSkeleton(engine::SceneWorld* pSceneWorld);

	void SetCameraController(engine::CameraController* pCameraController) { m_pCameraController = pCameraController; }

private:
	engine::CameraController* m_pCameraController = nullptr;
};

}

