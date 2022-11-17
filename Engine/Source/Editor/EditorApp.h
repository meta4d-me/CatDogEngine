#pragma once

#include "Application/IApplication.h"
#include "EditorImGuiContext.h"

#include <memory>
#include <vector>

namespace engine
{

class Window;
class RenderContext;
class Renderer;

}

struct ImGuiContext;

namespace editor
{

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

private:
	// Windows
	std::unique_ptr<engine::Window> m_pMainWindow;
	std::vector<std::unique_ptr<engine::Window>> m_pUserWindows;

	// ImGui
	std::unique_ptr<EditorImGuiContext> m_pEditorImGuiContext;

	// Rendering
	engine::RenderContext* m_pRenderContext;
	std::vector<std::unique_ptr<engine::Renderer>>  m_pRenderers;
};

}