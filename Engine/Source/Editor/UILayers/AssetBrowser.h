#include "EditorImGuiLayer.h"

namespace editor
{

class AssetBrowser : public EditorImGuiLayer
{
public:
	using EditorImGuiLayer::EditorImGuiLayer;
	virtual ~AssetBrowser();

	virtual void Init() override;
	virtual void Update() override;
};

}