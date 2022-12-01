#include "EditorImGuiLayer.h"

#include <inttypes.h>

namespace editor
{

class GameView : public EditorImGuiLayer
{
public:
	using EditorImGuiLayer::EditorImGuiLayer;
	virtual ~GameView();

	virtual void Init() override;
	virtual void Update() override;
	void OnResize();
};

}