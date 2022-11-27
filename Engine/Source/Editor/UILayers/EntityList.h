#include "EditorImGuiLayer.h"

namespace editor
{

class EntityList : public EditorImGuiLayer
{
public:
	using EditorImGuiLayer::EditorImGuiLayer;
	virtual ~EntityList();

	virtual void Init() override;
	virtual void Update() override;
};

}