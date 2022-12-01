#include "EditorImGuiLayer.h"

namespace editor
{

class Inspector : public EditorImGuiLayer
{
public:
	using EditorImGuiLayer::EditorImGuiLayer;
	virtual ~Inspector();

	virtual void Init() override;
	virtual void Update() override;
};

}