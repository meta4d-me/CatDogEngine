#include "TestNodeEditor.h"


// TODO : can use StringCrc to access other UILayers from ImGuiContextInstance.


namespace ed = ax::NodeEditor;

namespace editor
{

TestNodeEditor::~TestNodeEditor()
{
}

void TestNodeEditor::Init()
{
    ImGuizmo::SetGizmoSizeClipSpace(0.25f);

    ed::Config config;
    config.SettingsFile = "BasicInteraction.json";
    m_Context = ed::CreateEditor(&config);
}

void TestNodeEditor::Update()
{
    {
        auto& io = ImGui::GetIO();

        ImGui::Text("FPS: %.2f (%.2gms)", io.Framerate, io.Framerate ? 1000.0f / io.Framerate : 0.0f);

        ImGui::Separator();

        ed::SetCurrentEditor(m_Context);

        // Start interaction with editor.
        ed::Begin("My Editor", ImVec2(0.0, 0.0f));

        int uniqueId = 1;

        //
        // 1) Commit known data to editor
        //

        engine::SceneWorld* pSceneWorld = GetSceneWorld();
        ed::NodeId MaterialNode = uniqueId++;
        materialNodePinIds.clear();
        textureMap.clear();
        textureNodeId.clear();
        textureNodePinId.clear();

            for (int i = 0; i < 9; i++)
            {
                materialNodePinIds.push_back(uniqueId++);
            }

            for (auto& texture : pSceneWorld->GetSceneDatabase()->GetTextures())
            {
                textureNodeId.push_back(uniqueId++);
                textureNodePinId.push_back(uniqueId++);
                switch (texture.GetType())
                {
                case cd::MaterialTextureType::BaseColor:
                        textureTypeList.push_back(0);
                        break;
                case cd::MaterialTextureType::Occlusion:
                        textureTypeList.push_back(1);
                        break;
                case cd::MaterialTextureType::Roughness:
                        textureTypeList.push_back(2);
                        break;
                case cd::MaterialTextureType::Metallic:
                        textureTypeList.push_back(3);
                        break;
                case cd::MaterialTextureType::Normal:
                        textureTypeList.push_back(4);
                        break;
                case cd::MaterialTextureType::Emissive:
                        textureTypeList.push_back(5);
                        break;
                case cd::MaterialTextureType::Elevation:
                        textureTypeList.push_back(6);
                        break;
                case cd::MaterialTextureType::AlphaMap:
                        textureTypeList.push_back(7);
                        break;
                case cd::MaterialTextureType::General:
                        textureTypeList.push_back(8);
                        break;
                }
            }

            int numNodes = sizeof(textureNodeId) / sizeof(textureNodeId[0]);
        if (pSceneWorld->GetSceneDatabase()->GetTextureCount())
        {
            for (int i = 0; i < numNodes; i++)
            {
                if (m_FirstFrame)
                    ed::SetNodePosition(textureNodeId[i], ImVec2(1, i * 90.0f));
                ed::BeginNode(textureNodeId[i]);
                ImGui::Text("texture %d", i);
                ed::BeginPin(textureNodePinId[i], ed::PinKind::Output);
                ImGui::Text("Out ->");
                ed::EndPin();
                ed::EndNode();
            }
        }

        if (pSceneWorld->GetSceneDatabase()->GetMaterialCount())
        {
            if (m_FirstFrame)
                ed::SetNodePosition(MaterialNode, ImVec2(0, 90));
            ed::BeginNode(MaterialNode);
            ImGui::Text("Material ");
            ed::BeginPin(materialNodePinIds[0], ed::PinKind::Input);
            ImGui::Text(" BaseColor");
            ed::EndPin();
            ed::BeginPin(materialNodePinIds[1], ed::PinKind::Input);
            ImGui::Text(" Occlusion");
            ed::EndPin();
            ed::BeginPin(materialNodePinIds[2], ed::PinKind::Input);
            ImGui::Text(" Roughness");
            ed::EndPin();
            ed::BeginPin(materialNodePinIds[3], ed::PinKind::Input);
            ImGui::Text(" Metallic");
            ed::EndPin();
            ed::BeginPin(materialNodePinIds[4], ed::PinKind::Input);
            ImGui::Text(" Normal");
            ed::EndPin();
            ed::BeginPin(materialNodePinIds[5], ed::PinKind::Input);
            ImGui::Text(" Emissive");
            ed::EndPin();
            ed::BeginPin(materialNodePinIds[6], ed::PinKind::Input);
            ImGui::Text(" Elevation");
            ed::EndPin();
            ed::BeginPin(materialNodePinIds[7], ed::PinKind::Input);
            ImGui::Text(" AlphaMap");
            ed::EndPin();
            ed::BeginPin(materialNodePinIds[8], ed::PinKind::Input);
            ImGui::Text(" General");
            ed::EndPin();
            ed::EndNode();
        }

        if ((pSceneWorld->GetSceneDatabase()->GetMaterialCount() || pSceneWorld->GetSceneDatabase()->GetTextureCount() )&& linkSwitch)
        {
            for (int i = 0; i < numNodes; i++)
            {
                m_Links.push_back({ ed::LinkId(m_NextLinkId++), textureNodePinId[i], materialNodePinIds[textureTypeList[i]] });
                ed::Link(m_Links.back().Id, m_Links.back().InputId, m_Links.back().OutputId);
            }
            linkSwitch = false;
        }

        // Submit Links
        for (auto& linkInfo : m_Links)
            ed::Link(linkInfo.Id, linkInfo.InputId, linkInfo.OutputId);

        //
        // 2) Handle interactions
        //

        // Handle creation action, returns true if editor want to create new object (node or link)
        if (ed::BeginCreate())
        {
            ed::PinId inputPinId, outputPinId;
            if (ed::QueryNewLink(&inputPinId, &outputPinId))
            {
                // QueryNewLink returns true if editor want to create new link between pins.
                //
                // Link can be created only for two valid pins, it is up to you to
                // validate if connection make sense. Editor is happy to make any.
                //
                // Link always goes from input to output. User may choose to drag
                // link from output pin or input pin. This determine which pin ids
                // are valid and which are not:
                //   * input valid, output invalid - user started to drag new ling from input pin
                //   * input invalid, output valid - user started to drag new ling from output pin
                //   * input valid, output valid   - user dragged link over other pin, can be validated

                if (inputPinId && outputPinId) // both are valid, let's accept link
                {
                    // ed::AcceptNewItem() return true when user release mouse button.
                    if (ed::AcceptNewItem())
                    {
                        // Since we accepted new link, lets add one to our list of links.
                        m_Links.push_back({ ed::LinkId(m_NextLinkId++), inputPinId, outputPinId });

                        // Draw new link.
                        ed::Link(m_Links.back().Id, m_Links.back().InputId, m_Links.back().OutputId);
                    }

                    // You may choose to reject connection between these nodes
                    // by calling ed::RejectNewItem(). This will allow editor to give
                    // visual feedback by changing link thickness and color.
                }
            }
        }
        ed::EndCreate(); // Wraps up object creation action handling.


        // Handle deletion action
        if (ed::BeginDelete())
        {
            // There may be many links marked for deletion, let's loop over them.
            ed::LinkId deletedLinkId;
            while (ed::QueryDeletedLink(&deletedLinkId))
            {
                // If you agree that link can be deleted, accept deletion.
                if (ed::AcceptDeletedItem())
                {
                    // Then remove link from your data.
                    for (auto& link : m_Links)
                    {
                        if (link.Id == deletedLinkId)
                        {
                            m_Links.erase(&link);
                            break;
                        }
                    }
                }

                // You may reject link deletion by calling:
                // ed::RejectDeletedItem();
            }
        }
        ed::EndDelete(); // Wrap up deletion action

        // End of interaction with editor.
        ed::End();

        if (m_FirstFrame)
            ed::NavigateToContent(0.0f);

        ed::SetCurrentEditor(nullptr);

        m_FirstFrame = false;

        // ImGui::ShowMetricsWindow();
    }
}

}