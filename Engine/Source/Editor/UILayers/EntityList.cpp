#include "EntityList.h"

#include "ECWorld/EditorSceneWorld.h"
#include "ECWorld/World.h"
#include "ECWorld/CameraComponent.h"
#include "ECWorld/LightComponent.h"
#include "ECWorld/NameComponent.h"
#include "ECWorld/StaticMeshComponent.h"
#include "ECWorld/TransformComponent.h"

#include "ImGui/IconFont/IconsMaterialDesignIcons.h"

#include <imgui/imgui_internal.h>

namespace editor
{

EntityList::~EntityList()
{

}

void EntityList::Init()
{

}

void EntityList::AddEntity()
{
    engine::World* pWorld = m_pEditorSceneWorld->GetWorld();
    if (ImGui::Selectable("Add Mesh"))
    {
        engine::Entity meshEntity = pWorld->CreateEntity();
        pWorld->CreateComponent<engine::NameComponent>(meshEntity);
        pWorld->CreateComponent<engine::TransformComponent>(meshEntity);
        pWorld->CreateComponent<engine::StaticMeshComponent>(meshEntity);
    }
    else if (ImGui::Selectable("Add Camera"))
    {
        engine::Entity cameraEntity = pWorld->CreateEntity();
        pWorld->CreateComponent<engine::NameComponent>(cameraEntity);
        pWorld->CreateComponent<engine::TransformComponent>(cameraEntity);
        pWorld->CreateComponent<engine::CameraComponent>(cameraEntity);
    }
    else if (ImGui::Selectable("Add Light"))
    {
        engine::Entity lightEntity = pWorld->CreateEntity();
        pWorld->CreateComponent<engine::NameComponent>(lightEntity);
        pWorld->CreateComponent<engine::TransformComponent>(lightEntity);
        pWorld->CreateComponent<engine::StaticMeshComponent>(lightEntity);
        pWorld->CreateComponent<engine::LightComponent>(lightEntity);
    }
}

void EntityList::DrawEntity(engine::Entity entity)
{
    engine::World* pWorld = m_pEditorSceneWorld->GetWorld();
    engine::NameComponent* pNameComponent = pWorld->GetComponents<engine::NameComponent>()->GetComponent(entity);
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

    // TODO : selected or not?
     // ImGuiTreeNodeFlags_Selected
    ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_FramePadding |
        ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_SpanAvailWidth;

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
    if (pWorld->GetComponents<engine::CameraComponent>()->GetComponent(entity))
    {
        entityIcon = reinterpret_cast<const char*>(ICON_MDI_CAMERA);
    }
    else if (pWorld->GetComponents<engine::LightComponent>()->GetComponent(entity))
    {
        entityIcon = reinterpret_cast<const char*>(ICON_MDI_LIGHTBULB);
    }
    else if (pWorld->GetComponents<engine::StaticMeshComponent>()->GetComponent(entity))
    {
        entityIcon = reinterpret_cast<const char*>(ICON_MDI_SHAPE);
    }

    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_Text]);
    bool isNodeOpen = ImGui::TreeNodeEx(pNameComponent->GetName(), nodeFlags, "%s", entityIcon);
    ImGui::PopStyleColor();
    if (ImGui::IsItemClicked())
    {
        // Select it.
    }
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
            // pNameComponent->SetName();
        }

        ImGui::Separator();

        if (ImGui::Selectable("Delete"))
        {

        }

        ImGui::EndPopup();
    }

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

	auto flags = ImGuiWindowFlags_NoCollapse;
	ImGui::Begin(GetName(), &m_isEnable, flags);

	ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImGui::GetStyleColorVec4(ImGuiCol_TabActive));
    if (ImGui::Button(reinterpret_cast<const char*>(ICON_MDI_PLUS)))
    {
        ImGui::OpenPopup("AddEntity");
    }

    if (ImGui::BeginPopup("AddEntity"))
    {
        AddEntity();
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

    // TODO : Need to have a more generic entity manager.
    for (engine::Entity entity : m_pEditorSceneWorld->GetWorld()->GetComponents<engine::StaticMeshComponent>()->GetEntities())
    {
        DrawEntity(entity);
    }

    ImGui::Indent();

    ImGui::EndChild();

	ImGui::End();
}

}