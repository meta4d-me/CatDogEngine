#pragma once

#include "Core/EngineDefines.h"

#include <map>

namespace engine
{

class Object;

class CSharpBridge
{
public:
	using ObjectCreatorFunction = Object* (ObjectTypeGUID, void*, int);
	using ObjectPropertySetFunction = void (ObjectGUID, void*, int);
	using ObjectPropertyGetFunction = void (ObjectGUID, void**, int*);

	using ObjectCreatorFunctionMap = std::map<ObjectTypeGUID, ObjectCreatorFunction*>;
	using ObjectPropertyFunctions = std::pair<ObjectPropertySetFunction*, ObjectPropertyGetFunction*>;
	using ObjectPropertyFunctionMap = std::map<uint64_t, ObjectPropertyFunctions>;

public:
	CSharpBridge() = default;
	CSharpBridge(const CSharpBridge&) = delete;
	CSharpBridge& operator=(const CSharpBridge&) = delete;
	CSharpBridge(CSharpBridge&&) = delete;
	CSharpBridge& operator=(CSharpBridge&&) = delete;
	~CSharpBridge() = default;

	void RegisterObjectType(const char* pTypeName, ObjectCreatorFunction creatorFunc);
	void RegisterObjectTypeProperty(const char* pTypeName, const char* pPropertyName, ObjectPropertySetFunction setFunc, ObjectPropertyGetFunction getFunc);
	ObjectTypeGUID GetObjectTypeId(const char* pTypeName) const;
	ObjectPropertyUID GetObjectPropertyId(ObjectTypeGUID tid, const char* pPropertyName) const;

	ObjectGUID CreateObject(ObjectTypeGUID tid, void* pData, int size);
	void DestroyObject(ObjectTypeGUID tid, ObjectGUID instanceId);
	void SetObjectProperty(ObjectTypeGUID tid, ObjectPropertyUID pid, ObjectGUID instanceId, void* pData, int size);
	void GetObjectProperty(ObjectTypeGUID tid, ObjectPropertyUID pid, ObjectGUID instanceId, void** ppData, int* pSize);

private:
	ObjectCreatorFunctionMap m_mapObjectCreatorFunctions;
	ObjectPropertyFunctionMap m_mapObjectPropertyFunctions;
};

}