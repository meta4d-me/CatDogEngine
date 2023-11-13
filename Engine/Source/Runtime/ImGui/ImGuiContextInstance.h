#pragma once

#include "Localization.h"
#include "ThemeColor.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

struct ImGuiContext;

namespace engine
{

class ImGuiBaseLayer;
class RenderContext;
class SceneWorld;
class WindowManager;

class ImGuiContextInstance
{
public:
	ImGuiContextInstance() = delete;
	explicit ImGuiContextInstance(uint16_t width, uint16_t height, bool enableDock = false, bool enableViewport = false);
	ImGuiContextInstance(const ImGuiContextInstance&) = delete;
	ImGuiContextInstance& operator=(const ImGuiContextInstance&) = delete;
	ImGuiContextInstance(ImGuiContextInstance&&) = default;
	ImGuiContextInstance& operator=(ImGuiContextInstance&&) = default;
	virtual ~ImGuiContextInstance();

	void Update(float deltaTime);

	// Context settings.
	bool IsActive() const;
	void SwitchCurrentContext() const;

	// GUI management.
	void OnResize(uint16_t width, uint16_t height);
	void AddStaticLayer(std::unique_ptr<ImGuiBaseLayer> pLayer);
	std::vector<std::unique_ptr<ImGuiBaseLayer>>& GetDockableLayers() { return m_pImGuiDockableLayers; }
	void AddDynamicLayer(std::unique_ptr<ImGuiBaseLayer> pLayer);
	void ClearUILayers();

	// GUI styles.
	void LoadFontFiles(const std::vector<std::string>& ttfFileNames, engine::Language language);
	ThemeColor GetImGuiThemeColor() const { return m_themeColor; }
	void SetImGuiThemeColor(ThemeColor theme);
	Language GetImGuiLanguage() const { return m_language; }
	void SetImGuiLanguage(Language language);

	// Input management.
	void SetWindowPosOffset(float x, float y) { m_windowPosOffsetX = x; m_windowPosOffsetY = y; }

	// Viewport feature.
	void InitViewport(WindowManager* pWindowManager, RenderContext* pRenderContext);
	void UpdateViewport();

	// Access to world data.
	void SetSceneWorld(SceneWorld* pSceneWorld) { m_pSceneWorld = pSceneWorld; }
	SceneWorld* GetSceneWorld() const { return m_pSceneWorld; }

private:
	void AddInputEvent();
	void SetImGuiStyles();
	void BeginDockSpace();
	void EndDockSpace();

private:
	SceneWorld* m_pSceneWorld = nullptr;
	ImGuiContext* m_pImGuiContext = nullptr;
	Language m_language;
	ThemeColor m_themeColor;

	float m_windowPosOffsetX = 0.0f;
	float m_windowPosOffsetY = 0.0f;

	bool m_lastMouseLBPressed = false;
	bool m_lastMouseRBPressed = false;
	bool m_lastMouseMBPressed = false;
	float m_lastMouseScrollOffstY = 0.0f;
	float m_lastMousePositionX = 0.0f;
	float m_lastMousePositionY = 0.0f;

	std::vector<std::unique_ptr<ImGuiBaseLayer>> m_pImGuiStaticLayers;
	std::vector<std::unique_ptr<ImGuiBaseLayer>> m_pImGuiDockableLayers;
};

}