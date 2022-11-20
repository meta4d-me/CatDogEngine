#pragma once

#include <inttypes.h>

struct ImGuiContext;

namespace editor
{

class EditorImGuiContext
{
public:
	explicit EditorImGuiContext();
	EditorImGuiContext(const EditorImGuiContext&) = delete;
	EditorImGuiContext& operator=(const EditorImGuiContext&) = delete;
	EditorImGuiContext(EditorImGuiContext&&) = default;
	EditorImGuiContext& operator=(EditorImGuiContext&&) = default;
	virtual ~EditorImGuiContext();

	void Update();

	void OnMouseLBDown() { m_bLeftButtonDown = true; }
	void OnMouseLBUp() { m_bLeftButtonDown = false; }
	void OnMouseRBDown() { m_bRightButtonDown = true; }
	void OnMouseRBUp() { m_bRightButtonDown = false; }
	void OnMouseMBDown() { m_bMiddleButtonDown = true; }
	void OnMouseMBUp() { m_bMiddleButtonDown = false; }
	void OnMouseWheel(float offset) { m_mouseScollY = offset; }
	void OnMouseMove(int32_t x, int32_t y) { m_mouseX = x; m_mouseY = y; }

	//void OnKeyPress(int32_t keyCode, uint16_t mods);
	//void OnKeyRelease(int32_t keyCode, uint16_t mods);

private:
	ImGuiContext* m_pImGuiContext;

	int		m_mouseX = 0;
	int		m_mouseY = 0;
	bool	m_bLeftButtonDown = false;
	bool	m_bRightButtonDown = false;
	bool	m_bMiddleButtonDown = false;
	float	m_mouseScollY = 0.0f;

};

}