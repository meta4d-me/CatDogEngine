#pragma once

namespace engine
{

class ImGuiBaseLayer
{
public:
	ImGuiBaseLayer(const char* pName) : m_pName(pName) { }
	ImGuiBaseLayer(const ImGuiBaseLayer&) = delete;
	ImGuiBaseLayer& operator=(const ImGuiBaseLayer&) = delete;
	ImGuiBaseLayer(ImGuiBaseLayer&&) = default;
	ImGuiBaseLayer& operator=(ImGuiBaseLayer&&) = default;
	virtual ~ImGuiBaseLayer() = default;

	virtual void Init() = 0;
	virtual void Update() = 0;

	const char* GetName() const { return m_pName; }
	float GetWindowPosX() const { return m_windowPosX; }
	float GetWindowPosY() const { return m_windowPosY; }
	void SetWindowPos(float x, float y) { m_windowPosX = x; m_windowPosY = y; }

	void SetEnable(bool enable) { m_isEnable = enable; }
	bool IsEnable() const { return m_isEnable; }

protected:
	const char* m_pName = nullptr;
	float m_windowPosX = 0.0f;
	float m_windowPosY = 0.0f;
	bool m_isEnable = true;
};

}