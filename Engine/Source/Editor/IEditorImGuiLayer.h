#pragma once

namespace editor
{

class IEditorImGuiLayer
{
public:
	virtual void Init() = 0;
	virtual void BeginFrame() = 0;
	virtual void Update() = 0;
	virtual void EndFrame() = 0;
};

}