#pragma once

#include "Application/Localization.h"
#include "Preferences/ThemeColor.h"

#include <inttypes.h>
#include <memory>
#include <string>
#include <vector>

namespace editor
{

class EditorImGuiLayer;

class EditorImGuiContext
{
public:
	explicit EditorImGuiContext();
	EditorImGuiContext(const EditorImGuiContext&) = delete;
	EditorImGuiContext& operator=(const EditorImGuiContext&) = delete;
	EditorImGuiContext(EditorImGuiContext&&) = default;
	EditorImGuiContext& operator=(EditorImGuiContext&&) = default;
	virtual ~EditorImGuiContext();

	void SetImGuiStyles();
	ThemeColor GetImGuiThemeColor() const { return m_themeColor; }
	void SetImGuiThemeColor(ThemeColor theme);
	void LoadFontFiles(const std::vector<std::string>& ttfFileNames, engine::Language language) const;

	void AddStaticLayer(std::unique_ptr<EditorImGuiLayer> pLayer);

	std::vector<std::unique_ptr<EditorImGuiLayer>>& GetDockableLayers() { return m_pImGuiDockableLayers; }
	void AddDockableLayer(std::unique_ptr<EditorImGuiLayer> pLayer);
	
	void BeginDockSpace();
	void EndDockSpace();
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
	ThemeColor m_themeColor;

	std::vector<std::unique_ptr<EditorImGuiLayer>> m_pImGuiStaticLayers;
	std::vector<std::unique_ptr<EditorImGuiLayer>> m_pImGuiDockableLayers;
};

}