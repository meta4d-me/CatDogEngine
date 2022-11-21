#pragma once

#include "Application/IApplication.h"

#include <memory>
#include <vector>

namespace engine
{

class FlybyCamera;
class Window;
class RenderContext;
class Renderer;

}

struct ImGuiContext;

namespace editor
{

class EditorImGuiContext;
class EditorImGuiViewport;

class EditorApp final : public engine::IApplication
{
public:
	explicit EditorApp();
	EditorApp(const EditorApp&) = delete;
	EditorApp& operator=(const EditorApp&) = delete;
	EditorApp(EditorApp&&) = default;
	EditorApp& operator=(EditorApp&&) = default;
	virtual ~EditorApp();

	virtual void Init(engine::EngineInitArgs initArgs) override;
	virtual bool Update(float deltaTime) override;
	virtual void Shutdown() override;

	engine::Window* GetWindow(size_t index) const;
	engine::Window* GetMainWindow() const;
	size_t AddWindow(std::unique_ptr<engine::Window> pWindow);

	void InitRenderContext();
	void InitImGuiContext(engine::Language language);
	void InitImGuiViewports(engine::RenderContext* pRenderContext);

private:
	// Windows
	std::vector<std::unique_ptr<engine::Window>> m_pAllWindows;

	// ImGui
	std::unique_ptr<EditorImGuiContext> m_pEditorImGuiContext;
	std::unique_ptr<EditorImGuiViewport> m_pEditorImGuiViewport;

	// Rendering
	engine::RenderContext* m_pRenderContext;

	// TODO
	std::unique_ptr<engine::FlybyCamera> m_pCamera;
};

}