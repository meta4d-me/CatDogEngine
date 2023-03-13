#include "TerrainEditor.h"

#include "ECWorld/ECTerrainConsumer.h"
#include "ECWorld/SceneWorld.h"
#include "Framework/Processor.h"
#include "ImGui/IconFont/IconsMaterialDesignIcons.h"
#include "ImGui/ImGuiContextInstance.h"
#include "Rendering/RenderContext.h"
#include "Utilities/StringUtils.h"

#include <imgui/imgui.h>

using namespace cd;
using namespace cdtools;
using namespace engine;

namespace editor
{

TerrainEditor::TerrainEditor(const char* pName)
	: ImGuiBaseLayer(pName)
	, m_terrainMetadata(1, 1, 0, 2000, 5.0f)
	, m_sectorMetadata(1, 1, 10, 10)
	, m_terrainProducer(m_terrainMetadata, m_sectorMetadata)
	, m_terrainEntities()
{}

TerrainEditor::~TerrainEditor()
{

}

void TerrainEditor::Init()
{
	// Initialize some reasonable values
	m_terrainMetadata.numSectorsInX = 5;
	m_terrainMetadata.numSectorsInZ = 5;
	m_terrainMetadata.minElevation = 0;
	m_terrainMetadata.maxElevation = 200;
	m_terrainMetadata.redistPow = 5.0f;
	m_terrainMetadata.octaves.push_back(ElevationOctave(4564781151579, 1.0f, 1.0f));
	m_terrainMetadata.octaves.push_back(ElevationOctave(1341177459887, 2.0f, 0.8f));
	m_terrainMetadata.octaves.push_back(ElevationOctave(1551312147123, 4.0f, 0.6f));
	m_terrainMetadata.octaves.push_back(ElevationOctave(8707137379821, 8.0f, 0.4f));
	m_terrainMetadata.octaves.push_back(ElevationOctave(1362045344796, 16.0f, 0.2f));
	m_terrainMetadata.octaves.push_back(ElevationOctave(8714321343102, 32.0f, 0.1f));

	m_sectorMetadata.numQuadsInX = 5;
	m_sectorMetadata.numQuadsInZ = 5;
	m_sectorMetadata.quadLenInX = 20;
	m_sectorMetadata.quadLenInZ = 20;

	m_pSceneDatabase = std::make_unique<cd::SceneDatabase>();
}

void TerrainEditor::Update()
{
	static constexpr float kInputItemWidth = 120;

	auto flags = ImGuiWindowFlags_None; // ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
	ImGui::Begin(GetName(), &m_isEnable, flags);

	if (ImGui::Button("Update Terrain"))
	{
		ImGuiIO& io = ImGui::GetIO();
		RenderContext* pCurrentRenderContext = reinterpret_cast<RenderContext*>(io.BackendRendererUserData);
		ImGuiContextInstance* pImGuiContextInstance = reinterpret_cast<ImGuiContextInstance*>(io.UserData);
		SceneWorld* pSceneWorld = pImGuiContextInstance->GetSceneWorld();

		if (!m_pEcTerrainConsumer) {
			m_pEcTerrainConsumer = std::make_unique<ECTerrainConsumer>(pSceneWorld, pCurrentRenderContext);
		} else {
			// Clean up previously generated entities
			m_pSceneDatabase = std::make_unique<cd::SceneDatabase>();
			m_pEcTerrainConsumer->Clear();
		}
		m_terrainProducer.SetSceneDatabaseIDs(m_pSceneDatabase.get());
		m_terrainProducer.SetTerrainMetadata(m_terrainMetadata);
		m_terrainProducer.SetSectorMetadata(m_sectorMetadata);
		m_terrainProducer.Initialize();

		Processor processor(&m_terrainProducer, m_pEcTerrainConsumer.get(), m_pSceneDatabase.get());
		processor.Run();
	}
	// Terrain Metadata Group
	ImGui::BeginGroup();
	{
		const uint16_t sectorStep = 1;
		ImGui::Text("Terrain Parameters:");
		ImGui::SetNextItemWidth(kInputItemWidth);
		if (ImGui::InputScalar("Sector Count X", ImGuiDataType_U16, &m_terrainMetadata.numSectorsInX, &sectorStep, &sectorStep, NULL, ImGuiInputTextFlags_CharsDecimal))
		{
			if (m_terrainMetadata.numSectorsInX == 0)
			{
				m_terrainMetadata.numSectorsInX = 1;
			}
		}
		ImGui::SetNextItemWidth(kInputItemWidth);
		if (ImGui::InputScalar("Sector Count Z", ImGuiDataType_U16, &m_terrainMetadata.numSectorsInZ, &sectorStep, &sectorStep, NULL, ImGuiInputTextFlags_CharsDecimal))
		{
			if (m_terrainMetadata.numSectorsInZ == 0)
			{
				m_terrainMetadata.numSectorsInZ = 1;
			}
		}
		ImGui::SetNextItemWidth(kInputItemWidth);
		if (ImGui::InputScalar("Quads Count X", ImGuiDataType_U16, &m_sectorMetadata.numQuadsInX, &sectorStep, &sectorStep, NULL, ImGuiInputTextFlags_CharsDecimal))
		{
			if (m_sectorMetadata.numQuadsInX == 0)
			{
				m_sectorMetadata.numQuadsInX = 1;
			}
		}
		ImGui::SetNextItemWidth(kInputItemWidth);
		if (ImGui::InputScalar("Quads Count Z", ImGuiDataType_U16, &m_sectorMetadata.numQuadsInZ, &sectorStep, &sectorStep, NULL, ImGuiInputTextFlags_CharsDecimal))
		{
			if (m_sectorMetadata.numQuadsInZ == 0)
			{
				m_sectorMetadata.numQuadsInZ = 1;
			}
		}
		ImGui::SetNextItemWidth(kInputItemWidth);
		if (ImGui::InputScalar("Quads Length X", ImGuiDataType_U16, &m_sectorMetadata.quadLenInX, &sectorStep, &sectorStep, NULL, ImGuiInputTextFlags_CharsDecimal))
		{
			if (m_sectorMetadata.quadLenInX == 0)
			{
				m_sectorMetadata.quadLenInX = 1;
			}
		}
		ImGui::SetNextItemWidth(kInputItemWidth);
		if (ImGui::InputScalar("Quads Length Z", ImGuiDataType_U16, &m_sectorMetadata.quadLenInZ, &sectorStep, &sectorStep, NULL, ImGuiInputTextFlags_CharsDecimal))
		{
			if (m_sectorMetadata.quadLenInZ == 0)
			{
				m_sectorMetadata.quadLenInZ = 1;
			}
		}

		const int32_t elevationStep = 1;
		const int32_t elevationBigStep = 10;
		ImGui::SetNextItemWidth(kInputItemWidth);
		if (ImGui::InputScalar("Min Elevation", ImGuiDataType_S32, &m_terrainMetadata.minElevation, &elevationStep, &elevationBigStep, NULL, ImGuiInputTextFlags_CharsDecimal))
		{
			if (m_terrainMetadata.minElevation >= m_terrainMetadata.maxElevation)
			{
				m_terrainMetadata.minElevation = m_terrainMetadata.maxElevation - 1;
			}
		}
		ImGui::SetNextItemWidth(kInputItemWidth);
		if (ImGui::InputScalar("Max Elevation", ImGuiDataType_S32, &m_terrainMetadata.maxElevation, &elevationStep, &elevationBigStep, NULL, ImGuiInputTextFlags_CharsDecimal))
		{
			if (m_terrainMetadata.maxElevation <= m_terrainMetadata.minElevation)
			{
				m_terrainMetadata.maxElevation = m_terrainMetadata.minElevation + 1;
			}
		}

		const float redistPowStep = 0.1f;
		const float redistPowBigStep = 1.0f;
		ImGui::SetNextItemWidth(kInputItemWidth);
		ImGui::InputScalar("Power", ImGuiDataType_Float, &m_terrainMetadata.redistPow, &redistPowStep, &redistPowBigStep, "%.2f", ImGuiInputTextFlags_CharsDecimal);
	}
	ImGui::EndGroup();

	// Octaves Group
	const int64_t seedStep = 1;
	const float freqAndWeightStep = 0.1f;
	ImGui::BeginGroup();
	{
		ImGui::Separator();
		if (ImGui::Button("Add Octave"))
		{
			m_terrainMetadata.octaves.emplace_back();
		}
		if (ImGui::Button("Remove Octave"))
		{
			m_terrainMetadata.octaves.pop_back();
		}
		ImGui::Separator();
		ImGui::BeginChild("Elevation Octaves", ImVec2(0, 0), true);
		for (uint32_t i = 0; i < m_terrainMetadata.octaves.size(); ++i)
		{
			ImGui::PushID(i);
			ImGui::InputScalar(string_format("Seed %d", i).c_str(), ImGuiDataType_S64, &m_terrainMetadata.octaves[i].seed, &seedStep, &seedStep, NULL, ImGuiInputTextFlags_CharsDecimal);
			if (ImGui::InputScalar(string_format("Frequency %d", i).c_str(), ImGuiDataType_Float, &m_terrainMetadata.octaves[i].frequency, &freqAndWeightStep, &freqAndWeightStep, NULL, ImGuiInputTextFlags_CharsDecimal))
			{
				if (m_terrainMetadata.octaves[i].frequency <= 0.0f)
				{
					m_terrainMetadata.octaves[i].frequency = freqAndWeightStep;
				}
			}
			ImGui::InputScalar(string_format("Weight %d", i).c_str(), ImGuiDataType_Float, &m_terrainMetadata.octaves[i].weight, &freqAndWeightStep, &freqAndWeightStep, NULL, ImGuiInputTextFlags_CharsDecimal);
			ImGui::PopID();
			ImGui::Separator();
		}
		ImGui::EndChild();
	}
	ImGui::EndGroup();

	ImGui::End();
}

}