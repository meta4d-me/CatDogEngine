#include "ImGui/ImGuiBaseLayer.h"

#include "Scene/Bone.h"
#include "Scene/SceneDatabase.h"
#include "Scene/Skeleton.h"

namespace editor
{

class SkeletonView : public engine::ImGuiBaseLayer
{
public:
	using ImGuiBaseLayer::ImGuiBaseLayer;
	virtual ~SkeletonView();

	virtual void Init() override;
	virtual void Update() override;

	void DrawBone(cd::SceneDatabase* pSceneDatabase,const cd::Bone& Bone);
	void DrawSkeleton(engine::SceneWorld* pSceneWorld);
};

}

