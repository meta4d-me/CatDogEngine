#pragma once

#include "Core/EngineDefines.h"

typedef void(__stdcall* LogCallbackType)(int messageType, wchar_t* text);
typedef void(__stdcall* InvalidateViewsCallbackType)(void);

struct HitRecord;
struct FrameTime;
class RenderSurface;
class Ray;
enum UpdateTypeEnum;
enum VertexFormatEnum;
enum BasicRendererFlagsEnum;
enum PrimitiveTypeEnum;
typedef unsigned int FontStyleFlags;

extern "C"
{

// These APIs are copied from LevelEditor project. And we will leave the implementation empty at first to make level editor can work without crash.
// Then we need to implement necessary APIs for editor needs.
EDITOR_API void __stdcall LvEd_Initialize(LogCallbackType logCallback, InvalidateViewsCallbackType invalidateCallback, const wchar_t** outEngineInfo);
EDITOR_API void __stdcall LvEd_Shutdown();
EDITOR_API void __stdcall LvEd_Clear();
EDITOR_API ObjectTypeGUID __stdcall LvEd_GetObjectTypeId(char* className);
EDITOR_API ObjectPropertyUID __stdcall LvEd_GetObjectPropertyId(ObjectTypeGUID id, char* propertyName);
EDITOR_API ObjectPropertyUID __stdcall LvEd_GetObjectChildListId(ObjectTypeGUID id, char* listName);
EDITOR_API ObjectGUID  __stdcall LvEd_CreateObject(ObjectTypeGUID typeId, void* data, int size);
EDITOR_API void __stdcall LvEd_DestroyObject(ObjectTypeGUID typeId, ObjectGUID instanceId);
EDITOR_API void __stdcall LvEd_InvokeMemberFn(ObjectGUID instanceId, wchar_t* fn, const void* arg, void** retVal);
EDITOR_API void __stdcall LvEd_SetObjectProperty(ObjectTypeGUID typeId, ObjectPropertyUID propId, ObjectGUID instanceId, void* data, int size);
EDITOR_API void __stdcall LvEd_GetObjectProperty(ObjectTypeGUID typeId, ObjectPropertyUID propId, ObjectGUID instanceId, void** data, int* size);
EDITOR_API void __stdcall LvEd_ObjectAddChild(ObjectTypeGUID typeId, ObjectListUID listId, ObjectGUID parentId, ObjectGUID childId, int index);
EDITOR_API void __stdcall LvEd_ObjectRemoveChild(ObjectTypeGUID typeId, ObjectListUID listId, ObjectGUID parentId, ObjectGUID childId);
EDITOR_API bool __stdcall LvEd_RayPick(float viewxform[], float projxform[], Ray* rayW, bool skipSelected, HitRecord** hits, int* count);
EDITOR_API bool __stdcall LvEd_FrustumPick(ObjectGUID renderSurface, float viewxform[], float projxform[], float* rect, HitRecord** hits, int* count);
EDITOR_API void __stdcall LvEd_SetSelection(ObjectGUID* instanceIds, int count);
EDITOR_API void __stdcall LvEd_SetRenderState(ObjectGUID instId);
EDITOR_API void __stdcall LvEd_SetGameLevel(ObjectGUID instId);
EDITOR_API ObjectGUID __stdcall LvEd_GetGameLevel();
EDITOR_API void  __stdcall LvEd_WaitForPendingResources();
EDITOR_API void  __stdcall LvEd_Update(FrameTime* ft, UpdateTypeEnum updateType);
EDITOR_API void __stdcall LvEd_Begin(ObjectGUID renderSurface, float viewxform[], float projxform[]);
EDITOR_API void __stdcall LvEd_End();
EDITOR_API void __stdcall LvEd_RenderGame();
EDITOR_API bool __stdcall LvEd_SaveRenderSurfaceToFile(ObjectGUID renderSurfaceId, wchar_t* fileName);
EDITOR_API ObjectGUID __stdcall LvEd_CreateVertexBuffer(VertexFormatEnum vf, void* buffer, uint32_t vertexCount);
EDITOR_API ObjectGUID __stdcall LvEd_CreateIndexBuffer(uint32_t* buffer, uint32_t indexCount);
EDITOR_API void __stdcall LvEd_DeleteBuffer(ObjectGUID buffer);
EDITOR_API void __stdcall LvEd_SetRendererFlag(BasicRendererFlagsEnum renderFlags);
EDITOR_API void __stdcall LvEd_DrawPrimitive(PrimitiveTypeEnum pt, ObjectGUID vb, uint32_t StartVertex, uint32_t vertexCount, float* color, float* xform);
EDITOR_API void __stdcall LvEd_DrawIndexedPrimitive(PrimitiveTypeEnum pt, ObjectGUID vb, ObjectGUID ib, uint32_t startIndex, uint32_t indexCount, uint32_t startVertex, float* color, float* xform);
EDITOR_API ObjectGUID LvEd_CreateFont(wchar_t* fontName, float pixelHeight, FontStyleFlags fontStyles);
EDITOR_API void __stdcall LvEd_DeleteFont(ObjectGUID font);
EDITOR_API void LvEd_DrawText2D(ObjectGUID font, wchar_t* text, int x, int y, int color);
EDITOR_API int __stdcall LvEd_GetLastError(const wchar_t** errorText);

}