#include "ImGui/ImGuiBaseLayer.h"
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
	void SetSkeletonIsOpen(bool isOpen) { m_isSkeletonWidowOpen = isOpen; }
	bool GetSkeletonIsOpem() { return m_isSkeletonWidowOpen; }


private:
	bool m_isSkeletonWidowOpen = false;
};

}

