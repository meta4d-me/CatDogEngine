#pragma once

namespace engine
{

class RenderContext;

}

namespace editor
{

class EditorApp;

struct EditorImGuiViewportData
{
	engine::RenderContext* pRenderContext;
};

class EditorImGuiViewport
{
public:
	EditorImGuiViewport() = delete;
	explicit EditorImGuiViewport(editor::EditorApp* pEditor, engine::RenderContext* pRenderContext);
	EditorImGuiViewport(const EditorImGuiViewport&) = delete;
	EditorImGuiViewport& operator=(const EditorImGuiViewport&) = delete;
	EditorImGuiViewport(EditorImGuiViewport&&) = default;
	EditorImGuiViewport& operator=(EditorImGuiViewport&&) = default;

	void Update();
};

}