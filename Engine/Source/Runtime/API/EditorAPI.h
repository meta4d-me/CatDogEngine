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
ENGINE_API void __stdcall LvEd_Initialize(LogCallbackType logCallback, InvalidateViewsCallbackType invalidateCallback, const wchar_t** outEngineInfo);
ENGINE_API void __stdcall LvEd_Shutdown();
ENGINE_API void __stdcall LvEd_Clear();
ENGINE_API ObjectTypeGUID __stdcall LvEd_GetObjectTypeId(char* className);
ENGINE_API ObjectPropertyUID __stdcall LvEd_GetObjectPropertyId(ObjectTypeGUID id, char* propertyName);
ENGINE_API ObjectPropertyUID __stdcall LvEd_GetObjectChildListId(ObjectTypeGUID id, char* listName);
ENGINE_API ObjectGUID  __stdcall LvEd_CreateObject(ObjectTypeGUID typeId, void* data, int size);
ENGINE_API void __stdcall LvEd_DestroyObject(ObjectTypeGUID typeId, ObjectGUID instanceId);
ENGINE_API void __stdcall LvEd_InvokeMemberFn(ObjectGUID instanceId, wchar_t* fn, const void* arg, void** retVal);
ENGINE_API void __stdcall LvEd_SetObjectProperty(ObjectTypeGUID typeId, ObjectPropertyUID propId, ObjectGUID instanceId, void* data, int size);
ENGINE_API void __stdcall LvEd_GetObjectProperty(ObjectTypeGUID typeId, ObjectPropertyUID propId, ObjectGUID instanceId, void** data, int* size);
ENGINE_API void __stdcall LvEd_ObjectAddChild(ObjectTypeGUID typeId, ObjectListUID listId, ObjectGUID parentId, ObjectGUID childId, int index);
ENGINE_API void __stdcall LvEd_ObjectRemoveChild(ObjectTypeGUID typeId, ObjectListUID listId, ObjectGUID parentId, ObjectGUID childId);
ENGINE_API bool __stdcall LvEd_RayPick(float viewxform[], float projxform[], Ray* rayW, bool skipSelected, HitRecord** hits, int* count);
ENGINE_API bool __stdcall LvEd_FrustumPick(ObjectGUID renderSurface, float viewxform[], float projxform[], float* rect, HitRecord** hits, int* count);
ENGINE_API void __stdcall LvEd_SetSelection(ObjectGUID* instanceIds, int count);
ENGINE_API void __stdcall LvEd_SetRenderState(ObjectGUID instId);
ENGINE_API void __stdcall LvEd_SetGameLevel(ObjectGUID instId);
ENGINE_API ObjectGUID __stdcall LvEd_GetGameLevel();
ENGINE_API void  __stdcall LvEd_WaitForPendingResources();
ENGINE_API void  __stdcall LvEd_Update(FrameTime* ft, UpdateTypeEnum updateType);
ENGINE_API void __stdcall LvEd_Begin(ObjectGUID renderSurface, float viewxform[], float projxform[]);
ENGINE_API void __stdcall LvEd_End();
ENGINE_API void __stdcall LvEd_RenderGame();
ENGINE_API bool __stdcall LvEd_SaveRenderSurfaceToFile(ObjectGUID renderSurfaceId, wchar_t* fileName);
ENGINE_API ObjectGUID __stdcall LvEd_CreateVertexBuffer(VertexFormatEnum vf, void* buffer, uint32_t vertexCount);
ENGINE_API ObjectGUID __stdcall LvEd_CreateIndexBuffer(uint32_t* buffer, uint32_t indexCount);
ENGINE_API void __stdcall LvEd_DeleteBuffer(ObjectGUID buffer);
ENGINE_API void __stdcall LvEd_SetRendererFlag(BasicRendererFlagsEnum renderFlags);
ENGINE_API void __stdcall LvEd_DrawPrimitive(PrimitiveTypeEnum pt, ObjectGUID vb, uint32_t StartVertex, uint32_t vertexCount, float* color, float* xform);
ENGINE_API void __stdcall LvEd_DrawIndexedPrimitive(PrimitiveTypeEnum pt, ObjectGUID vb, ObjectGUID ib, uint32_t startIndex, uint32_t indexCount, uint32_t startVertex, float* color, float* xform);
ENGINE_API ObjectGUID LvEd_CreateFont(wchar_t* fontName, float pixelHeight, FontStyleFlags fontStyles);
ENGINE_API void __stdcall LvEd_DeleteFont(ObjectGUID font);
ENGINE_API void LvEd_DrawText2D(ObjectGUID font, wchar_t* text, int x, int y, int color);
ENGINE_API int __stdcall LvEd_GetLastError(const wchar_t** errorText);

}