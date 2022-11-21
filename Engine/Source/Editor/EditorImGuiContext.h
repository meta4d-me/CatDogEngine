#pragma once

#include "Application/Localization.h"

#include <inttypes.h>

struct ImGuiContext;
struct ImFont;

#include <memory>
#include <string>
#include <vector>

namespace editor
{

class IEditorImGuiLayer;

class EditorImGuiContext
{
public:
	explicit EditorImGuiContext();
	EditorImGuiContext(const EditorImGuiContext&) = delete;
	EditorImGuiContext& operator=(const EditorImGuiContext&) = delete;
	EditorImGuiContext(EditorImGuiContext&&) = default;
	EditorImGuiContext& operator=(EditorImGuiContext&&) = default;
	virtual ~EditorImGuiContext();

	void AddLayer(std::unique_ptr<IEditorImGuiLayer> pLayer);

	void SetImGuiStyles();
	void LoadFontFiles(const std::vector<std::string>& ttfFileNames, engine::Language language);
	void Update();

	void OnMouseLBDown();
	void OnMouseLBUp();
	void OnMouseRBDown();
	void OnMouseRBUp();
	void OnMouseMBDown();
	void OnMouseMBUp();
	void OnMouseWheel(float offset);
	void OnMouseMove(int32_t x, int32_t y);
	//void OnKeyPress(int32_t keyCode, uint16_t mods);
	//void OnKeyRelease(int32_t keyCode, uint16_t mods);

private:
	ImGuiContext* m_pImGuiContext;
	std::vector<std::unique_ptr<IEditorImGuiLayer>> m_pImGuiLayers;
};

}