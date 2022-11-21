#include "IEditorImGuiLayer.h"

namespace editor
{

class MainMenu : public IEditorImGuiLayer
{
public:
	virtual void Init() override;
	virtual void BeginFrame() override;
	virtual void Update() override;
	virtual void EndFrame() override;
};

}