#include "EditorImGuiLayer.h"

namespace editor
{

class OutputLog : public EditorImGuiLayer
{
public:
	using EditorImGuiLayer::EditorImGuiLayer;
	virtual ~OutputLog();

	virtual void Init() override;
	virtual void Update() override;
};

}