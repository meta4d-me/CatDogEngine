#include "DebugPanel.h"
#include "Display/CameraController.h"
#include "ImGui/IconFont/IconsMaterialDesignIcons.h"

#include <bgfx/bgfx.h>
#include <bx/string.h>
#include <bx/timer.h>
#include <bx/math.h>
#include <imgui/imgui.h>

namespace engine
{

namespace
{

struct SampleData
{
	static constexpr uint32_t SampleNum = 256;

	SampleData()
	{
		Reset();
	}

	void Reset()
	{
		m_offset = 0;
		bx::memSet(m_values, 0, sizeof(m_values));

		m_min = 0.0f;
		m_max = 0.0f;
		m_avg = 0.0f;
	}

	void PushSample(float value)
	{
		m_values[m_offset] = value;
		m_offset = (m_offset + 1) % SampleNum;

		float min = bx::max<float>();
		float max = bx::min<float>();
		float avg = 0.0f;

		for (uint32_t sampleIndex = 0; sampleIndex < SampleNum; ++sampleIndex)
		{
			const float val = m_values[sampleIndex];
			min = bx::min(min, val);
			max = bx::max(max, val);
			avg += val;
		}

		m_min = min;
		m_max = max;
		m_avg = avg / SampleNum;
	}

	int32_t m_offset;
	float m_values[SampleNum];

	float m_min;
	float m_max;
	float m_avg;
};

}

DebugPanel::~DebugPanel()
{

}

void DebugPanel::Init()
{

}

void DebugPanel::Update()
{
	ImGui::SetNextWindowSize(ImVec2(350, 200.0f));

	ImGui::Begin(GetName(), &m_isEnable);

	ShowProfiler();

	ImGui::Separator();

	if (ImGui::Selectable("Front"))
	{
		if (m_pCameraController)
			m_pCameraController->MoveToPosition(cd::Vec3f(-15.40f, 1.75f, 0.16f), cd::Vec3f(49.64f, 88.32f, 48.37f));
	}
	if (ImGui::Selectable("Left"))
	{
		if (m_pCameraController)
			m_pCameraController->MoveToPosition(cd::Vec3f(-2.40, 1.75f, -1.95f), cd::Vec3f(173.14f, 50.70f, 165.59f));
	}
	if (ImGui::Selectable("Right"))
	{
		if (m_pCameraController)
			m_pCameraController->MoveToPosition(cd::Vec3f(-4.71, 0.75f, 2.37f), cd::Vec3f(2.36f, 36.19f, 0.77f));
	}
	if (ImGui::Selectable("Behind"))
	{
		if (m_pCameraController)
			m_pCameraController->MoveToPosition(cd::Vec3f(9.18, 0.75f, -7.05f), cd::Vec3f(-25.74f, 33.71f, -7.92f));
	}

	ImGui::End();
}

void DebugPanel::ShowProfiler()
{
	static SampleData s_frameTime;
	
	const bgfx::Stats* stats = bgfx::getStats();
	float toMsCpu = 1000.0f / static_cast<float>(stats->cpuTimerFreq);
	float toMsGpu = 1000.0f / static_cast<float>(stats->gpuTimerFreq);
	float frameMs = static_cast<float>(stats->cpuTimeFrame) * toMsCpu;

	s_frameTime.PushSample(frameMs);

	char frameTextOverlay[256];
	bx::snprintf(frameTextOverlay, BX_COUNTOF(frameTextOverlay), "%s%.3fms, %s%.3fms\nAvg: %.3fms, %.1f FPS"
		, ICON_MDI_ARROW_DOWN
		, s_frameTime.m_min
		, ICON_MDI_ARROW_UP
		, s_frameTime.m_max
		, s_frameTime.m_avg
		, 1000.0f / s_frameTime.m_avg
	);
	
	ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImColor(0.0f, 0.5f, 0.15f, 1.0f).Value);
	ImGui::PlotHistogram("Frame"
		, s_frameTime.m_values
		, SampleData::SampleNum
		, s_frameTime.m_offset
		, frameTextOverlay
		, 0.0f
		, 60.0f
		, ImVec2(0.0f, 45.0f)
	);
	ImGui::PopStyleColor();
	
	ImGui::Text("Submit CPU %0.3f, GPU %0.3f (L: %d)"
		, static_cast<float>(stats->cpuTimeEnd - stats->cpuTimeBegin) * toMsCpu
		, static_cast<float>(stats->gpuTimeEnd - stats->gpuTimeBegin) * toMsGpu
		, stats->maxGpuLatency
	);
	
	if (-INT64_MAX != stats->gpuMemoryUsed)
	{
		char tmp0[64];
		bx::prettify(tmp0, BX_COUNTOF(tmp0), stats->gpuMemoryUsed);
	
		char tmp1[64];
		bx::prettify(tmp1, BX_COUNTOF(tmp1), stats->gpuMemoryMax);
	
		ImGui::Text("GPU mem: %s / %s", tmp0, tmp1);
	}
}

}