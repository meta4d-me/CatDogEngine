#include "Profiler.h"
#include "ImGui/IconFont/IconsMaterialDesignIcons.h"

#include <bgfx/bgfx.h>
#include <bx/string.h>
#include <bx/timer.h>
#include <bx/math.h>
#include <imgui/imgui.h>

#include <cmath>

namespace
{

bool DrawBar(const char* id, float width, float maxWidth, float height, const ImVec4& color)
{
    const ImGuiStyle& style = ImGui::GetStyle();

    ImVec4 hoveredColor(color.x * 1.1f, color.y * 1.1f, color.z * 1.1f, color.w * 1.1f);

    ImGui::PushStyleColor(ImGuiCol_Button, color);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hoveredColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, color);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, style.ItemSpacing.y));

    bool itemHovered = false;

    ImGui::PushID(id);

    ImGui::Button("##Visible", ImVec2(width, height));
    itemHovered = itemHovered || ImGui::IsItemHovered();

    ImGui::SameLine();
    ImGui::InvisibleButton("##Invisible", ImVec2(std::max(1.0f, maxWidth - width), height));
    itemHovered = itemHovered || ImGui::IsItemHovered();

    ImGui::PopID();

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(3);

    return itemHovered;
}

}

namespace engine
{

Profiler::~Profiler()
{
}

void Profiler::Init()
{
}

void Profiler::Update()
{
    // top left, transparent background
    //ImGui::SetNextWindowPos(padding, ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.5f);
    constexpr auto flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
    ImGui::Begin(GetName(), &m_isEnable, flags);

    const float overlayWidth = 150.0f;
    const ImVec2 padding = { 5.0f, 5.0f };

    float deltaTime = ImGui::GetIO().DeltaTime;
    static bool showFPS = true;
    static bool showFrameTime = true;
    static bool showViewStats = true;
    static bool showGPUMemory = true;

    // title
    ImGui::Text("Stats");
    ImGui::TextDisabled("right-click for options");
    ImGui::Separator();

    // general data
    const bgfx::Stats* stats = bgfx::getStats();
    const double toCpuMs = 1000.0 / double(stats->cpuTimerFreq);
    const double toGpuMs = 1000.0 / double(stats->gpuTimerFreq);

    ImGui::Text("Backend: %s", bgfx::getRendererName(bgfx::getRendererType()));
    ImGui::Text("Buffer size: %u x %u px", stats->width, stats->height);
    ImGui::Text("Triangles: %u", stats->numPrims[bgfx::Topology::TriList]);
    ImGui::Text("Draw calls: %u", stats->numDraw);
    ImGui::Text("Compute calls: %u", stats->numCompute);

    // plots
    static constexpr size_t GRAPH_HISTORY = 100;
    static float fpsValues[GRAPH_HISTORY] = { 0 };
    static float frameTimeValues[GRAPH_HISTORY] = { 0 };
    static float gpuMemoryValues[GRAPH_HISTORY] = { 0 };
    static size_t offset = 0;

    if (showFPS)
    {
        ImGui::Separator();
        ImGui::Text("FPS");
        ImGui::PlotLines("",
            fpsValues,
            IM_ARRAYSIZE(fpsValues),
            (int)offset + 1,
            nullptr,
            0.0f,
            200.0f,
            ImVec2(overlayWidth, 50));
        ImGui::Text("%.0f", fpsValues[offset]);
    }

    if (showFrameTime)
    {
        ImGui::Separator();
        ImGui::Text("Frame time");
        ImGui::PlotLines("",
            frameTimeValues,
            IM_ARRAYSIZE(frameTimeValues),
            (int)offset + 1,
            nullptr,
            0.0f,
            30.0f,
            ImVec2(overlayWidth, 50));
        ImGui::Text("CPU: %.2f ms", float(stats->cpuTimeEnd - stats->cpuTimeBegin) * toCpuMs);
        ImGui::Text("GPU: %.2f ms", float(stats->gpuTimeEnd - stats->gpuTimeBegin) * toGpuMs);
        ImGui::Text("Total: %.2f ms", frameTimeValues[offset]);
    }

    if (showViewStats)
    {
        ImGui::Separator();
        ImGui::Text("View stats");
        if (stats->numViews > 0)
        {
            ImVec4 cpuColor(0.5f, 1.0f, 0.5f, 1.0f);
            ImVec4 gpuColor(0.5f, 0.5f, 1.0f, 1.0f);

            const float itemHeight = ImGui::GetTextLineHeightWithSpacing();
            const float itemHeightWithSpacing = ImGui::GetFrameHeightWithSpacing();
            const float scale = 2.0f;

            if (ImGui::BeginListBox("##ViewStats", ImVec2(overlayWidth, stats->numViews * itemHeightWithSpacing)))
            {
                ImGuiListClipper clipper;
                clipper.Begin(stats->numViews, itemHeight);

                while (clipper.Step())
                {
                    for (int32_t pos = clipper.DisplayStart; pos < clipper.DisplayEnd; ++pos)
                    {
                        const bgfx::ViewStats& viewStats = stats->viewStats[pos];
                        float cpuElapsed = float((viewStats.cpuTimeEnd - viewStats.cpuTimeBegin) * toCpuMs);
                        float gpuElapsed = float((viewStats.gpuTimeEnd - viewStats.gpuTimeBegin) * toGpuMs);

                        ImGui::Text("%d", viewStats.view);

                        const float maxWidth = overlayWidth * 0.35f;
                        const float cpuWidth = bx::clamp(cpuElapsed * scale, 1.0f, maxWidth);
                        const float gpuWidth = bx::clamp(gpuElapsed * scale, 1.0f, maxWidth);

                        ImGui::SameLine(overlayWidth * 0.3f);

                        if (DrawBar("CPU", cpuWidth, maxWidth, itemHeight, cpuColor))
                        {
                            ImGui::SetTooltip("%s -- CPU: %.2f ms", viewStats.name, cpuElapsed);
                        }

                        ImGui::SameLine();

                        if (DrawBar("GPU", gpuWidth, maxWidth, itemHeight, gpuColor))
                        {
                            ImGui::SetTooltip("%s -- GPU: %.2f ms", viewStats.name, gpuElapsed);
                        }
                    }
                }

                clipper.End();

                ImGui::EndListBox();
            }
        }
        else
        {
            ImGui::TextWrapped("Profiler disabled");
        }
    }

    if (showGPUMemory)
    {
        int64_t used = stats->gpuMemoryUsed;
        int64_t max = stats->gpuMemoryMax;

        ImGui::Separator();
        if (used > 0 && max > 0)
        {
            ImGui::Text("GPU memory");
            ImGui::PlotLines("",
                gpuMemoryValues,
                IM_ARRAYSIZE(gpuMemoryValues),
                (int)offset + 1,
                nullptr,
                0.0f,
                float(max),
                ImVec2(overlayWidth, 50));
            char strUsed[64];
            bx::prettify(strUsed, BX_COUNTOF(strUsed), stats->gpuMemoryUsed);
            char strMax[64];
            bx::prettify(strMax, BX_COUNTOF(strMax), stats->gpuMemoryMax);
            ImGui::Text("%s / %s", strUsed, strMax);
        }
    }

    // update after drawing so offset is the current value
    static float currentTime = 0.0f;
    static float oldTime = 0.0f;
    constexpr float GRAPH_FREQUENCY = 0.1f;
    currentTime += deltaTime;
    if (currentTime - oldTime > GRAPH_FREQUENCY)
    {
        offset = (offset + 1) % GRAPH_HISTORY;
        ImGuiIO& io = ImGui::GetIO();
        fpsValues[offset] = 1 / io.DeltaTime;
        frameTimeValues[offset] = io.DeltaTime * 1000;
        gpuMemoryValues[offset] = float(stats->gpuMemoryUsed) / 1024 / 1024;

        oldTime = currentTime;
    }

    // right click menu
    if (ImGui::BeginPopupContextWindow())
    {
        ImGui::Checkbox("FPS", &showFPS);
        ImGui::Checkbox("Frame time", &showFrameTime);
        ImGui::Checkbox("View stats", &showViewStats);
        ImGui::Checkbox("GPU memory", &showGPUMemory);
        ImGui::EndPopup();
    }
    ImGui::End();
}

}