#include "IEditorImGuiLayer.h"

namespace editor
{

class DesignView : public IEditorImGuiLayer
{
public:
	virtual void Init() override;
	virtual void BeginFrame() override;
	virtual void Update() override;
	virtual void EndFrame() override;
};

}