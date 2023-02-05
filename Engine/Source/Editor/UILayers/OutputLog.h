#include "ImGui/ImGuiBaseLayer.h"

#include "imgui.h"

#include <sstream>

namespace editor
{

enum class LogLevel : uint8_t
{
	None = 0,
	Trace = 1 << 0,
	Info = 1 << 1,
	Warn = 1 << 2,
	Error = 1 << 3,
	Fatal = 1 << 4,
	All = 0xff,
};

class OutputLog : public engine::ImGuiBaseLayer
{
public:
	using ImGuiBaseLayer::ImGuiBaseLayer;
	virtual ~OutputLog();

	virtual void Init() override;
	virtual void Update() override;

	void Clear();
	void AddLog(const char *fmt, ...);
	void AddSpdLog(const std::ostringstream &oss, bool clearBuffer = true);
	void Draw();

private:
	void CreateButton(LogLevel level);

	const ImVec4 GetLevelColor(LogLevel level) const;
	const char *GetLevelIcon(LogLevel level) const;
	const std::string GetFilterStr() const;

	void SetOutputColor(std::string_view str) const;

	ImGuiTextBuffer m_buffer;
	ImGuiTextFilter m_fillter;
	// Index to lines offset. We maintain this with AddLog() calls.
	ImVector<int> m_lineOffsets;
	uint8_t m_levelFilter = static_cast<uint8_t>(LogLevel::All);
};

} // namespace editor
