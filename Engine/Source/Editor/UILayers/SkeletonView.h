#include "ImGui/ImGuiBaseLayer.h"

#include "Scene/Bone.h"
#include "Scene/SceneDatabase.h"
namespace editor
{

class SkeletonView : public engine::ImGuiBaseLayer
{
public:
	using ImGuiBaseLayer::ImGuiBaseLayer;
	virtual ~SkeletonView();

	virtual void Init() override;
	virtual void Update() override;

	void SkeletonWidow();

	void DrawBone(cd::SceneDatabase* pSceneDatabase,cd::Bone* pBone = nullptr);
	void DrawSkeleton(engine::SceneWorld* pSceneWorld, cd::Bone* root = nullptr);


private:
	bool m_isSkeletonWidowOpen = false;
};

}

