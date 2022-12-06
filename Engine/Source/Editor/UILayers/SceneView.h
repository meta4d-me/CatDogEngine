#include "EditorImGuiLayer.h"

#include <inttypes.h>

namespace ImGuizmo
{

enum OPERATION;

}

namespace editor
{

class SceneView : public EditorImGuiLayer
{
public:
	using EditorImGuiLayer::EditorImGuiLayer;
	virtual ~SceneView();

	virtual void Init() override;
	virtual void Update() override;
	void OnResize();

private:
	void UpdateToolMenuButtons();

private:
	bool m_option1;
	bool m_option2;
	ImGuizmo::OPERATION m_currentOperation;
};

}