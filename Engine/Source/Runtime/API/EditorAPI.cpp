#include "EditorAPI.h"

#include "CSharpBridge.h"
#include "Engine.h"
#include "Rendering/RenderContext.h"

#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_print.hpp"
using XmlDocument = rapidxml::xml_document<wchar_t>;
using XmlNode = rapidxml::xml_node<wchar_t>;
using XmlAttribute = rapidxml::xml_attribute<wchar_t>;
static std::wstring s_engineInfoXml;


static engine::Engine* s_pEngineInstance = nullptr;

namespace SwapChainBridge
{

struct SwapChainDescriptor
{
	void*	 nwh;
	uint16_t width;
	uint16_t height;
};

static void CreateIntance(ObjectTypeGUID tid, void* data, int size)
{
	assert(size == sizeof(SwapChainDescriptor));
	SwapChainDescriptor* pScd = static_cast<SwapChainDescriptor*>(data);
	engine::RenderContext* pRenderContext = s_pEngineInstance->GetRenderContext();
	uint8_t swapChainID = pRenderContext->CreateSwapChain(pScd->nwh, pScd->width, pScd->height);
	// pRenderContext->GetSwapChain(swapChainID);
}

}

EDITOR_API void __stdcall LvEd_Initialize(LogCallbackType logCallback, InvalidateViewsCallbackType invalidateCallback, const wchar_t** outEngineInfo)
{
	if(s_pEngineInstance)
	{
		return;
	}

	s_pEngineInstance = new engine::Engine();
	s_pEngineInstance->InitCSharpBridge();
	s_pEngineInstance->Init();

	// Pass xml formated engine information to editor
	XmlDocument doc;
	if(XmlNode* pDecl = doc.allocate_node(rapidxml::node_declaration))
	{
		pDecl->append_attribute(doc.allocate_attribute(L"version", L"1.0"));
		pDecl->append_attribute(doc.allocate_attribute(L"encoding", L"utf-8"));
		doc.append_node(pDecl);
	}

	if(XmlNode* pRoot = doc.allocate_node(rapidxml::node_element, L"EngineInfo"))
	{
		pRoot->append_attribute(doc.allocate_attribute(L"name", L"CatDogEngine"));
		doc.append_node(pRoot);
	}

	rapidxml::print(std::back_inserter(s_engineInfoXml), doc, 0);
	*outEngineInfo = s_engineInfoXml.c_str();

	// Binding runtime engine classes to C# environment.
	if(engine::CSharpBridge* pBridge = s_pEngineInstance->GetCSharpBridge())
	{
		pBridge->RegisterObjectType("SwapChain", &SwapChainBridge::CreateIntance);
	}
}

EDITOR_API void __stdcall LvEd_Shutdown()
{
	if (s_pEngineInstance)
	{
		delete s_pEngineInstance;
		s_pEngineInstance = nullptr;
	}
}

EDITOR_API void __stdcall LvEd_Clear()
{
}

EDITOR_API ObjectTypeGUID __stdcall LvEd_GetObjectTypeId(char* className)
{
	return 0U;
}

EDITOR_API ObjectPropertyUID __stdcall LvEd_GetObjectPropertyId(ObjectTypeGUID id, char* propertyName)
{
	return 0U;
}

EDITOR_API ObjectPropertyUID __stdcall LvEd_GetObjectChildListId(ObjectTypeGUID id, char* listName)
{
	return 0U;
}

EDITOR_API ObjectGUID  __stdcall LvEd_CreateObject(ObjectTypeGUID typeId, void* data, int size)
{
	return 0U;
}

EDITOR_API void __stdcall LvEd_DestroyObject(ObjectTypeGUID typeId, ObjectGUID instanceId)
{
}

EDITOR_API void __stdcall LvEd_InvokeMemberFn(ObjectGUID instanceId, wchar_t* fn, const void* arg, void** retVal)
{
}

EDITOR_API void __stdcall LvEd_SetObjectProperty(ObjectTypeGUID typeId, ObjectPropertyUID propId, ObjectGUID instanceId, void* data, int size)
{
}

EDITOR_API void __stdcall LvEd_GetObjectProperty(ObjectTypeGUID typeId, ObjectPropertyUID propId, ObjectGUID instanceId, void** data, int* size)
{
}

EDITOR_API void __stdcall LvEd_ObjectAddChild(ObjectTypeGUID typeId, ObjectListUID listId, ObjectGUID parentId, ObjectGUID childId, int index)
{
}

EDITOR_API void __stdcall LvEd_ObjectRemoveChild(ObjectTypeGUID typeId, ObjectListUID listId, ObjectGUID parentId, ObjectGUID childId)
{
}

EDITOR_API bool __stdcall LvEd_RayPick(float viewxform[], float projxform[], Ray* rayW, bool skipSelected, HitRecord** hits, int* count)
{
	return false;
}

EDITOR_API bool __stdcall LvEd_FrustumPick(ObjectGUID renderSurface, float viewxform[], float projxform[], float* rect, HitRecord** hits, int* count)
{
	return false;
}

EDITOR_API void __stdcall LvEd_SetSelection(ObjectGUID* instanceIds, int count)
{
}

EDITOR_API void __stdcall LvEd_SetRenderState(ObjectGUID instId)
{
}

EDITOR_API void __stdcall LvEd_SetGameLevel(ObjectGUID instId)
{
}

EDITOR_API ObjectGUID __stdcall LvEd_GetGameLevel()
{
	return 0U;
}

EDITOR_API void  __stdcall LvEd_WaitForPendingResources()
{
}

EDITOR_API void  __stdcall LvEd_Update(FrameTime* ft, UpdateTypeEnum updateType)
{
}

EDITOR_API void __stdcall LvEd_Begin(ObjectGUID renderSurface, float viewxform[], float projxform[])
{
}

EDITOR_API void __stdcall LvEd_End()
{
}

EDITOR_API void __stdcall LvEd_RenderGame()
{
}

EDITOR_API bool __stdcall LvEd_SaveRenderSurfaceToFile(ObjectGUID renderSurfaceId, wchar_t* fileName)
{
	return false;
}

EDITOR_API ObjectGUID __stdcall LvEd_CreateVertexBuffer(VertexFormatEnum vf, void* buffer, uint32_t vertexCount)
{
	return 0U;
}

EDITOR_API ObjectGUID __stdcall LvEd_CreateIndexBuffer(uint32_t* buffer, uint32_t indexCount)
{
	return 0U;
}

EDITOR_API void __stdcall LvEd_DeleteBuffer(ObjectGUID buffer)
{
}

EDITOR_API void __stdcall LvEd_SetRendererFlag(BasicRendererFlagsEnum renderFlags)
{
}

EDITOR_API void __stdcall LvEd_DrawPrimitive(PrimitiveTypeEnum pt, ObjectGUID vb, uint32_t StartVertex, uint32_t vertexCount, float* color, float* xform)
{
}

EDITOR_API void __stdcall LvEd_DrawIndexedPrimitive(PrimitiveTypeEnum pt, ObjectGUID vb, ObjectGUID ib, uint32_t startIndex, uint32_t indexCount, uint32_t startVertex, float* color, float* xform)
{
}

EDITOR_API ObjectGUID LvEd_CreateFont(wchar_t* fontName, float pixelHeight, FontStyleFlags fontStyles)
{
	return 0U;
}

EDITOR_API void __stdcall LvEd_DeleteFont(ObjectGUID font)
{
}

EDITOR_API void LvEd_DrawText2D(ObjectGUID font, wchar_t* text, int x, int y, int color)
{
}

EDITOR_API int __stdcall LvEd_GetLastError(const wchar_t** errorText)
{
	return 0;
}
