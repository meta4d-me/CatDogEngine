#include "OutputLog.h"

#include "Base/Template.h"
#include "ImGui/IconFont/IconsMaterialDesignIcons.h"
#include "Log/Log.h"

#include <imgui/imgui.h>

namespace editor
{

namespace
{
constexpr ImVec4 COLOR_GREY =   { 0.9f, 0.9f, 0.9f, 1.0f };
constexpr ImVec4 COLOR_GREEN =  { 0.2f, 0.8f, 0.2f, 1.0f };
constexpr ImVec4 COLOR_BLUE =   { 0.2f, 0.2f, 0.8f, 1.0f };
constexpr ImVec4 COLOR_YELLOW = { 0.8f, 0.8f, 0.2f, 1.0f };
constexpr ImVec4 COLOR_RED =    { 0.8f, 0.25f, 0.25f, 1.0f };
constexpr ImVec4 COLOR_PURPLE = { 0.75f, 0.25f, 0.8f, 1.0f };
}

OutputLog::~OutputLog()
{

}

void OutputLog::Init()
{
	Clear();
}

void OutputLog::Update()
{
	ImGui::Begin(GetName(), &m_isEnable, 0);

#ifdef SPDLOG_ENABLE
    AddSpdLog(engine::Log::GetSpdOutput());
#endif
    Draw();

	ImGui::End();
}

void OutputLog::Clear() {
	m_buffer.clear();
	m_lineOffsets.clear();
	m_lineOffsets.push_back(0);
}

void OutputLog::AddLog(const char *fmt, ...) {
    int old_size = m_buffer.size();
    va_list args;
    va_start(args, fmt);
    m_buffer.appendfv(fmt, args);
    va_end(args);
    for (int new_size = m_buffer.size(); old_size < new_size; old_size++) {
        if (m_buffer[old_size] == '\n') {
            m_lineOffsets.push_back(old_size + 1);
        }
    }
}

void OutputLog::AddSpdLog(const std::ostringstream &oss, bool clearBuffer)
{
#ifdef SPDLOG_ENABLE
    AddLog(oss.str().c_str());
    if (clearBuffer) {
        engine::Log::ClearBuffer();
    }
#endif
}

void OutputLog::Draw() {
    // Main window.
    CreateButton(LogLevel::Trace);
    ImGui::SameLine();
    CreateButton(LogLevel::Info);
    ImGui::SameLine();
    CreateButton(LogLevel::Warn);
    ImGui::SameLine();
    CreateButton(LogLevel::Error);
    ImGui::SameLine();
    CreateButton(LogLevel::Fatal);

    ImGui::SameLine();
    bool clearFilter = ImGui::Button("Clear Filter");
    ImGui::SameLine();
    m_fillter.Draw("Filter");
    ImGui::Separator();

    if (ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar)) {
        if (clearFilter)
        {
            m_levelFilter = static_cast<uint8_t>(LogLevel::All);
            strcpy_s(m_fillter.InputBuf, "");
            m_fillter.Build();
        }

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        const char *buf = m_buffer.begin();
        const char *buf_end = m_buffer.end();
        if (m_fillter.IsActive()) {
            // In this example we don't use the clipper when Filter is enabled.
            // This is because we don't have random access to the result of our filter.
            // A real application processing logs with ten of thousands of entries may want to store the result of
            // search/filter.. especially if the filtering function is not trivial (e.g. reg-exp).
            for (int line_no = 0; line_no < m_lineOffsets.Size; line_no++) {
                const char *line_start = buf + m_lineOffsets[line_no];
                const char *line_end = (line_no + 1 < m_lineOffsets.Size) ? (buf + m_lineOffsets[line_no + 1] - 1) : buf_end;
                if (m_fillter.PassFilter(line_start, line_end)) {
                    SetOutputColor(std::string_view(line_start, line_end - line_start));
                    ImGui::TextUnformatted(line_start, line_end);
                    ImGui::PopStyleColor();
                }
            }
        }
        else {
            // The simplest and easy way to display the entire buffer:
            //   ImGui::TextUnformatted(buf_begin, buf_end);
            // And it'll just work. TextUnformatted() has specialization for large blob of text and will fast-forward
            // to skip non-visible lines. Here we instead demonstrate using the clipper to only process lines that are
            // within the visible area.
            // If you have tens of thousands of items and their processing cost is non-negligible, coarse clipping them
            // on your side is recommended. Using ImGuiListClipper requires
            // - A) random access into your data
            // - B) items all being the  same height,
            // both of which we can handle since we have an array pointing to the beginning of each line of text.
            // When using the filter (in the block of code above) we don't have random access into the data to display
            // anymore, which is why we don't use the clipper. Storing or skimming through the search result would make
            // it possible (and would be recommended if you want to search through tens of thousands of entries).
            ImGuiListClipper clipper;
            clipper.Begin(m_lineOffsets.Size);
            while (clipper.Step()) {
                for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++) {
                    const char *line_start = buf + m_lineOffsets[line_no];
                    const char *line_end = (line_no + 1 < m_lineOffsets.Size) ? (buf + m_lineOffsets[line_no + 1] - 1) : buf_end;

                    SetOutputColor(std::string_view(line_start, line_end - line_start));
                    ImGui::TextUnformatted(line_start, line_end);
                    ImGui::PopStyleColor();
                }
            }
            clipper.End();
        }
        ImGui::PopStyleVar();

        // Keep up at the bottom of the scroll region if we were already at the bottom at the beginning of the frame.
        // Using a scrollbar or mouse-wheel will take away from the bottom edge.
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
            ImGui::SetScrollHereY(1.0f);
        }
    }
    ImGui::EndChild();
}

void OutputLog::CreateButton(LogLevel level) {
    const bool isActive = m_levelFilter & static_cast<uint8_t>(level);
    if (isActive) {
        ImGui::PushStyleColor(ImGuiCol_Text, GetLevelColor(level));
    }
    else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5, 0.5f, 0.5f));
    }
    if (ImGui::Button(GetLevelIcon(level))) {
        m_levelFilter ^= static_cast<uint8_t>(level);
        strcpy_s(m_fillter.InputBuf, GetFilterStr().c_str());
        m_fillter.Build();
    }
    ImGui::PopStyleColor();
}

const ImVec4 OutputLog::GetLevelColor(LogLevel level) const {
    switch (level) {
        case LogLevel::Trace:
            return COLOR_GREY;
        case LogLevel::Info:
            return COLOR_GREEN;
        case LogLevel::Warn:
            return COLOR_YELLOW;
        case LogLevel::Error:
            return COLOR_RED;
        case LogLevel::Fatal:
            return COLOR_PURPLE;
        default:
            return { 0.5f, 0.5f , 0.5f , 0.5f };
    }
}

const char *OutputLog::GetLevelIcon(LogLevel level) const {
    switch (level) {
        case LogLevel::Trace:
            return reinterpret_cast<const char *>(ICON_MDI_MESSAGE_TEXT);
        case LogLevel::Info:
            return reinterpret_cast<const char *>(ICON_MDI_INFORMATION);
        case LogLevel::Warn:
            return reinterpret_cast<const char *>(ICON_MDI_ALERT);
        case LogLevel::Error:
            return reinterpret_cast<const char *>(ICON_MDI_CLOSE_CIRCLE);
        case LogLevel::Fatal:
            return reinterpret_cast<const char *>(ICON_MDI_BUG);
        default:
            return reinterpret_cast<const char *>(ICON_MDI_HELP);
    }
}

const std::string OutputLog::GetFilterStr() const {
    std::stringstream ss;
    if (m_levelFilter & static_cast<uint8_t>(LogLevel::Trace)) {
        ss << "trace,";
    }
    if (m_levelFilter & static_cast<uint8_t>(LogLevel::Info)) {
        ss << "info,";
    }
    if (m_levelFilter & static_cast<uint8_t>(LogLevel::Warn)) {
        ss << "warn,";
    }
    if (m_levelFilter & static_cast<uint8_t>(LogLevel::Error)) {
        ss << "error,";
    }
    if (m_levelFilter & static_cast<uint8_t>(LogLevel::Fatal)) {
        ss << "critical,";
    }
    return ss.str();
}

void OutputLog::SetOutputColor(std::string_view str) const {
    // TODO :
    // Spd just output information to some specific sink after formated,
    // whithout any extra information such as log level.
    // I can't find a better way to determine which level is for every log,
    // if we read buffer from spdlog to imgui.
    // Since the log formate will always be "[%T] [%n] [%l]: %v",
    // we can search log level start from the last "[" position.
    static const size_t startPoint = str.find_last_of("[");
    
    if (str.find("trace", startPoint) != str.npos) {
        ImGui::PushStyleColor(ImGuiCol_Text, COLOR_GREY);
    }
    else if (str.find("info", startPoint) != str.npos) {
        ImGui::PushStyleColor(ImGuiCol_Text, COLOR_GREEN);
    }
    else if (str.find("warn", startPoint) != str.npos) {
        ImGui::PushStyleColor(ImGuiCol_Text, COLOR_YELLOW);
    }
    else if (str.find("erro", startPoint) != str.npos) {
        ImGui::PushStyleColor(ImGuiCol_Text, COLOR_RED);
    }
    else if (str.find("critical", startPoint) != str.npos) {
        ImGui::PushStyleColor(ImGuiCol_Text, COLOR_PURPLE);
    }
    else {
        ImGui::PushStyleColor(ImGuiCol_Text, COLOR_GREY);
    }
}

} // namespace editor
