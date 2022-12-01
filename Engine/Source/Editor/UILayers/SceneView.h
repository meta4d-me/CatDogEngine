#include "EditorImGuiLayer.h"

#include <inttypes.h>

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
};

}