#include "EditorImGuiLayer.h"

namespace editor
{

class MainMenu : public EditorImGuiLayer
{
public:
	using EditorImGuiLayer::EditorImGuiLayer;
	virtual ~MainMenu();

	virtual void Init() override;
	virtual void Update() override;

	void FileMenu();
	void EditMenu();
	void WindowMenu();
	void AboutMenu();
};

}