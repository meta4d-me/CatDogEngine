#include "ImGui/ImGuiBaseLayer.h"

#include "ECWorld/Entity.h"
#include "Producers/TerrainProducer/AlphaMapTypes.h"
#include "Producers/TerrainProducer/TerrainProducer.h"
#include "Producers/TerrainProducer/TerrainTypes.h"

#include <vector>

namespace cd
{
class SceneDatabase;
}

namespace editor
{

	class ECTerrainConsumer;

class TerrainEditor : public engine::ImGuiBaseLayer
{
public:
	using ImGuiBaseLayer::ImGuiBaseLayer;

	explicit TerrainEditor(const char* pName);
	virtual ~TerrainEditor();

	virtual void Init() override;
	virtual void Update() override;

private:
	cdtools::TerrainMetadata m_terrainMetadata;
	cdtools::TerrainSectorMetadata m_sectorMetadata;
	bool m_generateAlphaMap;
	cdtools::AlphaMapBlendRegion<int32_t> m_redGreenBlendRegion;
	cdtools::AlphaMapBlendRegion<int32_t> m_greenBlueBlendRegion;
	cdtools::AlphaMapBlendRegion<int32_t> m_blueAlphaBlendRegion;
	char m_redChannelTextureName[128];
	char m_greenChannelTextureName[128];
	char m_blueChannelTextureName[128];
	char m_alphaChannelTextureName[128];
	cdtools::TerrainProducer m_terrainProducer;
	std::unique_ptr<ECTerrainConsumer> m_pEcTerrainConsumer;
	std::vector<engine::Entity> m_terrainEntities;
	std::unique_ptr<cd::SceneDatabase> m_pSceneDatabase;
};

}