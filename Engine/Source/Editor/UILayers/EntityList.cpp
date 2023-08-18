#include "EntityList.h"

#include "Display/CameraController.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/World.h"
#include "ImGui/IconFont/IconsMaterialDesignIcons.h"
#include "ImGui/ImGuiContextInstance.h"
#include "Math/MeshGenerator.h"
#include "Math/Sphere.hpp"
#include "Rendering/RenderContext.h"

#include <bgfx/bgfx.h>
#include <imgui/imgui_internal.h>

namespace cd
{

static std::optional<Mesh> Generate(uint32_t width, uint32_t depth, const VertexFormat& vertexFormat) /*HeightMap*/
{
    assert(vertexFormat.Contains(VertexAttributeType::Position));

    std::vector<cd::Point> positions;
    positions.reserve(width * depth);
    for (uint32_t z = 0U; z < depth; z++) {
        for (uint32_t x = 0U; x < width; x++) {
            positions.push_back(cd::Point(x, 0, z));
        }
    }

    std::vector<cd::Polygon> polygons;
    uint32_t NumQuads = (width - 1) * (depth - 1);
    polygons.reserve(NumQuads * 2);

    for (uint32_t z = 1U; z < depth - 1; z += 2) {
        for (uint32_t x = 1U; x < width - 1; x += 2) {
            uint32_t IndexCenter = z * width + x;

            uint32_t IndexTemp1 = (z - 1) * width + x - 1;
            uint32_t IndexTemp2 = z * width + x - 1;

            polygons.push_back(cd::Polygon{IndexCenter, IndexTemp1, IndexTemp2});

            IndexTemp1 = IndexTemp2;
            IndexTemp2 += width;
            polygons.push_back(cd::Polygon{IndexCenter, IndexTemp1, IndexTemp2});

            IndexTemp1 = IndexTemp2;
            IndexTemp2++;
            polygons.push_back(cd::Polygon{IndexCenter, IndexTemp1, IndexTemp2});

            IndexTemp1 = IndexTemp2;
            IndexTemp2++;
            polygons.push_back(cd::Polygon{IndexCenter, IndexTemp1, IndexTemp2});

            IndexTemp1 = IndexTemp2;
            IndexTemp2 -= width;
            polygons.push_back(cd::Polygon{IndexCenter, IndexTemp1, IndexTemp2});

            IndexTemp1 = IndexTemp2;
            IndexTemp2 -= width;
            polygons.push_back(cd::Polygon{IndexCenter, IndexTemp1, IndexTemp2});

            IndexTemp1 = IndexTemp2;
            IndexTemp2--;
            polygons.push_back(cd::Polygon{IndexCenter, IndexTemp1, IndexTemp2});

            IndexTemp1 = IndexTemp2;
            IndexTemp2--;
            polygons.push_back(cd::Polygon{IndexCenter, IndexTemp1, IndexTemp2});
        }
    }

    cd::Mesh mesh(static_cast<uint32_t>(positions.size()), static_cast<uint32_t>(polygons.size()));

    for (uint32_t i = 0U; i < positions.size(); ++i)
    {
        mesh.SetVertexPosition(i, positions[i]);
    }

    for (uint32_t i = 0U; i < polygons.size(); ++i)
    {
        mesh.SetPolygon(i, polygons[i]);
    }

    cd::VertexFormat meshVertexFormat;
    meshVertexFormat.AddAttributeLayout(cd::VertexAttributeType::Position, cd::GetAttributeValueType<cd::Point::ValueType>(), cd::Point::Size);

    if (vertexFormat.Contains(VertexAttributeType::Normal))
    {
        mesh.ComputeVertexNormals();
        meshVertexFormat.AddAttributeLayout(cd::VertexAttributeType::Normal, cd::GetAttributeValueType<cd::Direction::ValueType>(), cd::Direction::Size);
    }

    if (vertexFormat.Contains(VertexAttributeType::UV))
    {
        mesh.SetVertexUVSetCount(1);
        for (uint32_t vertexIndex = 0U; vertexIndex < mesh.GetVertexCount(); ++vertexIndex)
        {
            const auto& position = mesh.GetVertexPosition(vertexIndex);
            mesh.SetVertexUV(0U, vertexIndex, cd::UV(position.x() / 4, position.z() / 4));
        }

        meshVertexFormat.AddAttributeLayout(cd::VertexAttributeType::UV, cd::GetAttributeValueType<cd::UV::ValueType>(), cd::UV::Size);
    }

    if (vertexFormat.Contains(VertexAttributeType::Tangent) || vertexFormat.Contains(VertexAttributeType::Bitangent))
    {
        mesh.ComputeVertexTangents();
        meshVertexFormat.AddAttributeLayout(cd::VertexAttributeType::Tangent, cd::GetAttributeValueType<cd::Direction::ValueType>(), cd::Direction::Size);
        meshVertexFormat.AddAttributeLayout(cd::VertexAttributeType::Bitangent, cd::GetAttributeValueType<cd::Direction::ValueType>(), cd::Direction::Size);
    }

    // Use VertexColor0 to present braycentric coordinates.
    if (vertexFormat.Contains(VertexAttributeType::Color))
    {
        mesh.SetVertexColorSetCount(1U);
        meshVertexFormat.AddAttributeLayout(cd::VertexAttributeType::Color, cd::GetAttributeValueType<cd::Vec4f::ValueType>(), cd::Vec4f::Size);
    }

    mesh.SetVertexFormat(MoveTemp(meshVertexFormat));
    mesh.SetAABB(AABB(cd::Point(0, 0, 0), cd::Point(width, 0, depth)));

    return mesh;
}

}

namespace editor
{

EntityList::~EntityList()
{

}

void EntityList::Init()
{
}

void EntityList::AddEntity(engine::SceneWorld* pSceneWorld)
{
    engine::World* pWorld = pSceneWorld->GetWorld();
    cd::SceneDatabase* pSceneDatabase = pSceneWorld->GetSceneDatabase();
    engine::MaterialType* pPBRMaterialType = pSceneWorld->GetPBRMaterialType();
    engine::MaterialType* pTerrainMaterialType = pSceneWorld->GetTerrainMaterialType();

    auto AddNamedEntity = [&pWorld](std::string defaultName) -> engine::Entity
    {
        engine::Entity entity = pWorld->CreateEntity();
        auto& nameComponent = pWorld->CreateComponent<engine::NameComponent>(entity);
        nameComponent.SetName(defaultName + std::to_string(entity));

        return entity;
    };

    auto CreateShapeComponents = [&pSceneWorld, &pWorld, &pSceneDatabase](engine::Entity entity, cd::Mesh&& mesh, engine::MaterialType* pMaterialType)
    {
        auto& meshComponent = pWorld->CreateComponent<engine::StaticMeshComponent>(entity);
        meshComponent.SetMeshData(&mesh);
        meshComponent.SetRequiredVertexFormat(&pMaterialType->GetRequiredVertexFormat());
        meshComponent.Build();

        mesh.SetName(pSceneWorld->GetNameComponent(entity)->GetName());
        mesh.SetID(cd::MeshID(pSceneDatabase->GetMeshCount()));
        pSceneDatabase->AddMesh(cd::MoveTemp(mesh));

        auto& materialComponent = pWorld->CreateComponent<engine::MaterialComponent>(entity);
        materialComponent.Init();
        materialComponent.SetMaterialType(pMaterialType);
        materialComponent.SetAlbedoColor(cd::Vec3f(0.2f));
        materialComponent.SetSkyType(pSceneWorld->GetSkyComponent(pSceneWorld->GetSkyEntity())->GetSkyType());
        materialComponent.Build();

        auto& transformComponent = pWorld->CreateComponent<engine::TransformComponent>(entity);
        transformComponent.SetTransform(cd::Transform::Identity());
        transformComponent.Build();
    };

    auto CreateLightComponents = [&pWorld](engine::Entity entity, cd::LightType lightType, float intensity, cd::Vec3f color) -> engine::LightComponent&
    {
        auto& lightComponent = pWorld->CreateComponent<engine::LightComponent>(entity);
        lightComponent.SetType(lightType);
        lightComponent.SetIntensity(intensity);
        lightComponent.SetColor(color);

        auto& transformComponent = pWorld->CreateComponent<engine::TransformComponent>(entity);
        transformComponent.SetTransform(cd::Transform::Identity());
        transformComponent.Build();

        return lightComponent;
    };

    // ---------------------------------------- Add Mesh ---------------------------------------- //

    if (ImGui::MenuItem("Add Cube Mesh"))
    {
        engine::Entity entity = AddNamedEntity("CubeMesh");
        std::optional<cd::Mesh> optMesh = cd::MeshGenerator::Generate(cd::Box(cd::Point(-10.0f), cd::Point(10.0f)), pPBRMaterialType->GetRequiredVertexFormat());
        assert(optMesh.has_value());
        CreateShapeComponents(entity, cd::MoveTemp(optMesh.value()), pPBRMaterialType);
    }
    else if (ImGui::MenuItem("Add Sphere Mesh"))
    {
        engine::Entity entity = AddNamedEntity("Sphere");
        std::optional<cd::Mesh> optMesh = cd::MeshGenerator::Generate(cd::Sphere(cd::Point(0.0f), 10.0f), 100U, 100U, pPBRMaterialType->GetRequiredVertexFormat());
        assert(optMesh.has_value());
        CreateShapeComponents(entity, cd::MoveTemp(optMesh.value()), pPBRMaterialType);
    }
    else if (ImGui::MenuItem("Add Terrain Mesh"))
    {
        uint32_t TerrainSize = 129U;
        uint32_t HeightMapSize = 129U;//must be 129 now

        engine::Entity entity = AddNamedEntity("Terrain");

        std::optional<cd::Mesh> optMesh = cd::Generate(TerrainSize, TerrainSize, pTerrainMaterialType->GetRequiredVertexFormat());
        assert(optMesh.has_value());
        cd::Mesh& mesh = optMesh.value();
        auto& meshComponent = pWorld->CreateComponent<engine::StaticMeshComponent>(entity);
        meshComponent.SetMeshData(&mesh);
        meshComponent.SetRequiredVertexFormat(&pTerrainMaterialType->GetRequiredVertexFormat());//to do : modify vertexFormat
        meshComponent.Build();
        mesh.SetName(pSceneWorld->GetNameComponent(entity)->GetName());
        mesh.SetID(cd::MeshID(pSceneDatabase->GetMeshCount()));
        pSceneDatabase->AddMesh(cd::MoveTemp(mesh));

        auto& terrainComponent = pWorld->CreateComponent<engine::TerrainComponent>(entity);
        terrainComponent.SetWidth(HeightMapSize);
        terrainComponent.SetDepth(HeightMapSize);
        terrainComponent.InitElevationRawData();

        auto& materialComponent = pWorld->CreateComponent<engine::MaterialComponent>(entity);
        materialComponent.Init();
        materialComponent.SetMaterialType(pTerrainMaterialType);
        materialComponent.SetAlbedoColor(cd::Vec3f(0.2f));
        materialComponent.SetSkyType(pSceneWorld->GetSkyComponent(pSceneWorld->GetSkyEntity())->GetSkyType());
        materialComponent.SetTwoSided(true);
        materialComponent.Build();

        auto& transformComponent = pWorld->CreateComponent<engine::TransformComponent>(entity);
        transformComponent.SetTransform(cd::Transform::Identity());
        transformComponent.Build();
    }

    // ---------------------------------------- Add Camera ---------------------------------------- //

    else if (ImGui::MenuItem("Add Camera"))
    {
        engine::Entity entity = AddNamedEntity("Camera");
        auto& transformComponent = pWorld->CreateComponent<engine::TransformComponent>(entity);
        transformComponent.SetTransform(cd::Transform::Identity());
        transformComponent.Build();

        auto& cameraComponent = pWorld->CreateComponent<engine::CameraComponent>(entity);
        cameraComponent.SetAspect(1.0f);
        cameraComponent.SetFov(45.0f);
        cameraComponent.SetNearPlane(0.1f);
        cameraComponent.SetFarPlane(2000.0f);
        cameraComponent.SetNDCDepth(bgfx::getCaps()->homogeneousDepth ? cd::NDCDepth::MinusOneToOne : cd::NDCDepth::ZeroToOne);
        cameraComponent.BuildProjectMatrix();
        cameraComponent.BuildViewMatrix(cd::Transform::Identity());
    }

    // ---------------------------------------- Add Light ---------------------------------------- //

    else if (ImGui::MenuItem("Add Point Light"))
    {
        engine::Entity entity = AddNamedEntity("PointLight");
        auto& lightComponent = CreateLightComponents(entity, cd::LightType::Point, 1024.0f, cd::Vec3f(1.0f, 0.0f, 0.0f));
        lightComponent.SetPosition(cd::Point(0.0f, 0.0f, -16.0f));
        lightComponent.SetRange(1024.0f);
    }
    else if (ImGui::MenuItem("Add Spot Light"))
    {
        engine::Entity entity = AddNamedEntity("SpotLight");
        auto& lightComponent = CreateLightComponents(entity, cd::LightType::Spot, 1024.0f, cd::Vec3f(0.0f, 1.0f, 0.0f));
        lightComponent.SetPosition(cd::Point(0.0f, 0.0f, -16.0f));
        lightComponent.SetDirection(cd::Direction(0.0f, 0.0f, 1.0f));
        lightComponent.SetRange(1024.0f);
        lightComponent.SetInnerAndOuter(24.0f, 40.0f);
    }
    else if (ImGui::MenuItem("Add Directional Light"))
    {
        engine::Entity entity = AddNamedEntity("DirectionalLight");
        auto& lightComponent = CreateLightComponents(entity, cd::LightType::Directional, 4.0f, cd::Vec3f(1.0f, 1.0f, 1.0f));
        lightComponent.SetDirection(cd::Direction(0.0f, 0.0f, 1.0f));
    }

    // ---------------------------------------- Add Area Light ---------------------------------------- //

    else if (ImGui::MenuItem("Add Rectangle Light"))
    {
        engine::Entity entity = AddNamedEntity("RectangleLight");
        auto& lightComponent = CreateLightComponents(entity, cd::LightType::Rectangle, 1024.0f, cd::Vec3f(1.0f, 0.0f, 0.0f));
        lightComponent.SetPosition(cd::Point(0.0f, 0.0f, -12.0f));
        lightComponent.SetDirection(cd::Direction(0.0f, 0.0f, 1.0f));
        lightComponent.SetUp(cd::Direction(0.0f, 1.0f, 0.0f));
        lightComponent.SetRange(1024.0f);
        lightComponent.SetWidth(10.0f);
        lightComponent.SetHeight(10.0f);
    }
    else if (ImGui::MenuItem("Add Disk Light"))
    {
        engine::Entity entity = AddNamedEntity("DiskLight");
        auto& lightComponent = CreateLightComponents(entity, cd::LightType::Disk, 1024.0f, cd::Vec3f(1.0f, 0.0f, 0.0f));
        lightComponent.SetPosition(cd::Point(0.0f, 0.0f, -12.0f));
        lightComponent.SetDirection(cd::Direction(0.0f, 0.0f, 1.0f));
        lightComponent.SetRange(1024.0f);
        lightComponent.SetRadius(1024.0f);
    }
    else if (ImGui::MenuItem("Add Sphere Light"))
    {
        engine::Entity entity = AddNamedEntity("SphereLight");
        auto& lightComponent = CreateLightComponents(entity, cd::LightType::Sphere, 1024.0f, cd::Vec3f(1.0f, 0.0f, 0.0f));

        lightComponent.SetPosition(cd::Point(0.0f, 0.0f, -12.0f));
        lightComponent.SetDirection(cd::Direction(0.0f, 0.0f, 1.0f));
        lightComponent.SetRadius(1024.0f);
    }
    else if (ImGui::MenuItem("Add Tube Light"))
    {
        engine::Entity entity = AddNamedEntity("TubeLight");
        auto& lightComponent = CreateLightComponents(entity, cd::LightType::Tube, 1024.0f, cd::Vec3f(1.0f, 0.0f, 0.0f));
        lightComponent.SetPosition(cd::Point(0.0f, 0.0f, -12.0f));
        lightComponent.SetDirection(cd::Direction(0.0f, 0.0f, 1.0f));
        lightComponent.SetRange(1024.0f);
        lightComponent.SetWidth(10.0f);
    }
}

void EntityList::DrawEntity(engine::SceneWorld* pSceneWorld, engine::Entity entity)
{
    engine::NameComponent* pNameComponent = pSceneWorld->GetNameComponent(entity);
    if (!pNameComponent)
    {
        // Entity list requires entity to have name component to display.
        return;
    }

    // When you use the entity filter, it will skip drawing entites which doesn't pass the filter conditions.
    if (m_entityFilter.IsActive() && !m_entityFilter.PassFilter(pNameComponent->GetName()))
    {
        return;
    }

    ImGui::PushID(static_cast<int>(entity));

    ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_FramePadding |
        ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_SpanAvailWidth;
    if (entity == pSceneWorld->GetSelectedEntity())
    {
        nodeFlags |= ImGuiTreeNodeFlags_Selected;
    }

    // TODO : hierarchy or not?
    bool hasNoChildren = true;
    if (hasNoChildren)
    {
        nodeFlags |= ImGuiTreeNodeFlags_Leaf;
    }

    // TODO : active or not;
    bool isEntityActive = true;
    if (!isEntityActive)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
    }

    const char* entityIcon = reinterpret_cast<const char*>(ICON_MDI_CUBE_OUTLINE);
    if (pSceneWorld->GetCameraComponent(entity))
    {
        entityIcon = reinterpret_cast<const char*>(ICON_MDI_CAMERA);
    }
    else if (pSceneWorld->GetLightComponent(entity))
    {
        entityIcon = reinterpret_cast<const char*>(ICON_MDI_LIGHTBULB);
    }
    else if (pSceneWorld->GetStaticMeshComponent(entity))
    {
        entityIcon = reinterpret_cast<const char*>(ICON_MDI_SHAPE);
    }

    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_Text]);
    bool isNodeOpen = ImGui::TreeNodeEx(pNameComponent->GetName(), nodeFlags, "%s", entityIcon);
    ImGui::PopStyleColor();

    ImGui::SameLine();

    ImGui::Selectable(pNameComponent->GetName());
    
    if (!isEntityActive)
    {
        ImGui::PopStyleColor();
    }

    if (ImGui::BeginPopupContextItem(pNameComponent->GetName()))
    {
        if (ImGui::Selectable(CD_TEXT("Add Child")))
        {
        }

        if (ImGui::Selectable(CD_TEXT("Rename")))
        {
            // m_editingEntityName = true;
        }

        if (ImGui::Selectable(CD_TEXT("Delete")))
        {
            pSceneWorld->DeleteEntity(entity);
            pSceneWorld->GetSceneDatabase()->UpdateAABB();
        }

        // Operation list only for camera entites.
        engine::CameraComponent* pCameraComponent = pSceneWorld->GetCameraComponent(entity);
        if (pCameraComponent && entity != pSceneWorld->GetMainCameraEntity())
        {
            if (ImGui::Selectable(CD_TEXT("Set Main Camera")))
            {
                pSceneWorld->SetMainCameraEntity(entity);

                constexpr engine::StringCrc sceneRenderTarget("SceneRenderTarget");
                engine::RenderTarget* pRenderTarget = GetRenderContext()->GetRenderTarget(sceneRenderTarget);
                pCameraComponent->SetAspect(pRenderTarget->GetAspect());
            }
        }

        ImGui::EndPopup();
    }

    if (ImGui::IsItemClicked())
    {
        pSceneWorld->SetSelectedEntity(entity);
        if (ImGui::IsMouseDoubleClicked(0))
        {
            if (engine::StaticMeshComponent* pStaticMesh = pSceneWorld->GetStaticMeshComponent(entity))
            {
                cd::AABB meshAABB = pStaticMesh->GetAABB();
                if (engine::TransformComponent* pTransform = pSceneWorld->GetTransformComponent(entity))
                {
                    meshAABB = meshAABB.Transform(pTransform->GetWorldMatrix());
                    if (m_pCameraController)
                    {
                        m_pCameraController->CameraFocus(meshAABB);
                    }
                }
            }
        }
    }

    

    //if (m_editingEntityName)
    //{
    //    static char entityNewName[128];
    //    strcpy_s(entityNewName, pNameComponent->GetName());
    //    ImGui::PushItemWidth(-1);
    //    if (ImGui::InputText("##Name", entityNewName, IM_ARRAYSIZE(entityNewName), 0))
    //    {
    //        pNameComponent->SetName(entityNewName);
    //        m_editingEntityName = false;
    //    }
    //}

    if (!isNodeOpen)
    {
        ImGui::PopID();
        return;
    }

    const ImColor TreeLineColor(128, 128, 128, 128);
    constexpr float SmallOffsetX = 6.0f;
    ImVec2 verticalLineStart = ImGui::GetCursorScreenPos();
    verticalLineStart.x += SmallOffsetX; // to nicely line up with the arrow symbol
    ImVec2 verticalLineEnd = verticalLineStart;
    ImGui::GetWindowDrawList()->AddLine(verticalLineStart, verticalLineEnd, TreeLineColor);

    ImGui::TreePop();
    ImGui::PopID();
}

void EntityList::Update()
{
    engine::SceneWorld* pSceneWorld = GetSceneWorld();

    auto flags = ImGuiWindowFlags_NoCollapse;
    ImGui::Begin(GetName(), &m_isEnable, flags);

    ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImGui::GetStyleColorVec4(ImGuiCol_TabActive));
    if (ImGui::Button(reinterpret_cast<const char*>(ICON_MDI_PLUS)))
    {
        ImGui::OpenPopup("AddEntity");
    }

    if (ImGui::BeginPopup("AddEntity"))
    {
        AddEntity(pSceneWorld);
        ImGui::EndPopup();
    }

    ImGui::SameLine();
    ImGui::TextUnformatted(reinterpret_cast<const char*>(ICON_MDI_MAGNIFY));
    ImGui::SameLine();

    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(0, 0, 0, 0));
    m_entityFilter.Draw("##EntityFilter", ImGui::GetContentRegionAvail().x - ImGui::GetStyle().IndentSpacing);
    auto* drawList = ImGui::GetWindowDrawList();

    ImRect expandedRect = ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
    expandedRect.Min.x -= 1.0f;
    expandedRect.Min.y -= 1.0f;
    expandedRect.Max.x += 1.0f;
    expandedRect.Max.y += 1.0f;

    if (ImGui::IsItemActive())
    {
        drawList->AddRect(expandedRect.Min, expandedRect.Max, ImColor(80, 80, 80), 2.0f, 0, 1.0f);
    }
    else
    {
        if (ImGui::IsItemHovered())
        {
            drawList->AddRect(expandedRect.Min, expandedRect.Max, ImColor(60, 60, 60), 2.0f, 0, 1.5f);
        }
    }

    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
    ImGui::PopFont();

    if (!m_entityFilter.IsActive())
    {
        ImGui::SameLine();
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
        ImGui::SetCursorPosX(ImGui::GetFontSize() * 4.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, ImGui::GetStyle().FramePadding.y));
        ImGui::TextUnformatted("Search...");
        ImGui::PopStyleVar();
        ImGui::PopFont();
    }

    ImGui::PopStyleColor();
    ImGui::Unindent();
    ImGui::Separator();

    ImGui::BeginChild("Entites");

    for (engine::Entity entity : pSceneWorld->GetNameEntities())
    {
        DrawEntity(pSceneWorld, entity);
    }

    ImGui::Indent();

    ImGui::EndChild();

    ImGui::End();
}

}
