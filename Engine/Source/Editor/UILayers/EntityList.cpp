#include "EntityList.h"

#include "ECWorld/SceneWorld.h"
#include "ECWorld/World.h"
#include "ImGui/IconFont/IconsMaterialDesignIcons.h"
#include "ImGui/ImGuiContextInstance.h"
#include "Math/MeshGenerator.h"

#include <imgui/imgui_internal.h>

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

    auto AddNamedEntity = [&pWorld, &pSceneWorld](std::string defaultName) -> engine::Entity
    {
        engine::Entity entity = pWorld->CreateEntity();
        auto& nameComponent = pWorld->CreateComponent<engine::NameComponent>(entity);
        nameComponent.SetName(defaultName + std::to_string(entity));

        auto& transformComponent = pWorld->CreateComponent<engine::TransformComponent>(entity);
        transformComponent.SetTransform(cd::Transform::Identity());
        transformComponent.Build();

        return entity;
    };

    if (ImGui::Selectable("Add Mesh"))
    {
        engine::Entity entity = AddNamedEntity("Untitled_Mesh");
        engine::MaterialType* pPBRMaterialType = pSceneWorld->GetPBRMaterialType();
        std::optional<cd::Mesh> optMesh = cd::MeshGenerator::Generate(cd::Box(cd::Point(-10.0f), cd::Point(10.0f)), pPBRMaterialType->GetRequiredVertexFormat());
        assert(optMesh.has_value());

        auto& meshComponent = pWorld->CreateComponent<engine::StaticMeshComponent>(entity);
        meshComponent.SetMeshData(&optMesh.value());
        meshComponent.SetRequiredVertexFormat(&pPBRMaterialType->GetRequiredVertexFormat());
        meshComponent.Build();

        auto& materialComponent = pWorld->CreateComponent<engine::MaterialComponent>(entity);
        materialComponent.SetMaterialData(nullptr);
        materialComponent.SetMaterialType(pPBRMaterialType);
        materialComponent.SetUberShaderOption(engine::ShaderSchema::DefaultUberOption);
        materialComponent.Build();
    }
    else if (ImGui::Selectable("Add Camera"))
    {
        engine::Entity entity = AddNamedEntity("Untitled_Camera");
        pWorld->CreateComponent<engine::CameraComponent>(entity);
    }
    else if (ImGui::Selectable("Add Light"))
    {
        engine::Entity entity = AddNamedEntity("Untitled_Light");
        pWorld->CreateComponent<engine::LightComponent>(entity);
    }
}

void EntityList::DrawEntity(engine::SceneWorld* pSceneWorld, engine::Entity entity)
{
    engine::NameComponent* pNameComponent = pSceneWorld->GetNameComponent(entity);
    if (!pNameComponent)
    {
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
    ImGui::TextUnformatted(pNameComponent->GetName());

    if (!isEntityActive)
    {
        ImGui::PopStyleColor();
    }

    if (ImGui::BeginPopupContextItem(pNameComponent->GetName()))
    {
        if (ImGui::Selectable("Add Child"))
        {

        }
        else if (ImGui::Selectable("Rename"))
        {
           // m_editingEntityName = true;
        }
        else if (ImGui::Selectable("Delete"))
        {
            pSceneWorld->DeleteEntity(entity);
        }

        ImGui::EndPopup();
    }

    if (ImGui::IsItemClicked())
    {
        pSceneWorld->SetSelectedEntity(entity);
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
    ImGuiIO& io = ImGui::GetIO();
    engine::ImGuiContextInstance* pImGuiContextInstance = reinterpret_cast<engine::ImGuiContextInstance*>(io.UserData);
    engine::SceneWorld* pSceneWorld = pImGuiContextInstance->GetSceneWorld();

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

    ImGui::PushFont(io.Fonts->Fonts[0]);
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
        ImGui::PushFont(io.Fonts->Fonts[0]);
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

    for (engine::Entity entity : pSceneWorld->GetTransformEntities())
    {
        DrawEntity(pSceneWorld, entity);
    }

    ImGui::Indent();

    ImGui::EndChild();

	ImGui::End();
}

}