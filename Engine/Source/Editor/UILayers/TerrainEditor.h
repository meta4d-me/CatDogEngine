#include "ImGui/ImGuiBaseLayer.h"

#include "ECWorld/Entity.h"
#include "Producers/TerrainProducer/TerrainProducer.h"
#include "Producers/TerrainProducer/TerrainTypes.h"

#include <vector>

namespace cdtools
{
class TerrainProducer;
}

namespace editor
{

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
	cdtools::TerrainProducer m_terrainProducer;
	std::vector<engine::Entity> m_terrainEntities;
};

}