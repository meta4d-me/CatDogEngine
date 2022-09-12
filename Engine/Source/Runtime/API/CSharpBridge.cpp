#include "CSharpBridge.h"

#include "Utility/Hasher.h"

#include <cassert>

namespace
{

uint64_t MakeU64(uint32_t p1, uint32_t p2)
{
	return (uint64_t)p1 << 32 | p2;
}

}

namespace engine
{

void CSharpBridge::RegisterObjectType(const char* pTypeName, ObjectCreatorFunction* pCreatorFunc)
{
	ObjectTypeGUID tid = Hasher::Hash32(pTypeName);
	assert(m_mapObjectCreatorFunctions.find(tid) == m_mapObjectCreatorFunctions.end() && "Don't allow to register duplicated object type.");
	m_mapObjectCreatorFunctions[tid] = pCreatorFunc;
}

void CSharpBridge::RegisterObjectTypeProperty(const char* pTypeName, const char* pPropertyName, ObjectPropertySetFunction* pSetFunc, ObjectPropertyGetFunction* pGetFunc)
{
	ObjectTypeGUID tid = Hasher::Hash32(pTypeName);
	assert(m_mapObjectCreatorFunctions.find(tid) != m_mapObjectCreatorFunctions.end() && "The object type should be registered firstly.");

	ObjectPropertyUID pid = Hasher::Hash32(pPropertyName);
	uint64_t tpid = MakeU64(tid, pid);
	assert(m_mapObjectPropertyFunctions.find(tpid) == m_mapObjectPropertyFunctions.end() && "Don't allow to register duplicated object property.");
	m_mapObjectPropertyFunctions[tpid] = ObjectPropertyFunctions(pSetFunc, pGetFunc);
}

ObjectTypeGUID CSharpBridge::GetObjectTypeId(const char* pTypeName) const
{
	ObjectTypeGUID tid = Hasher::Hash32(pTypeName);
	assert(m_mapObjectCreatorFunctions.find(tid) != m_mapObjectCreatorFunctions.end() && "Failed to get unregistered object type id.");
	return tid;
}

ObjectPropertyUID CSharpBridge::GetObjectPropertyId(ObjectTypeGUID tid, const char* pPropertyName) const
{
	ObjectPropertyUID pid = Hasher::Hash32(pPropertyName);
	uint64_t tpid = MakeU64(tid, pid);
	assert(m_mapObjectCreatorFunctions.find(tid) != m_mapObjectCreatorFunctions.end() && "Failed to get unregistered object property id.");
	return pid;
}

ObjectGUID CSharpBridge::CreateObject(ObjectTypeGUID tid, void* pData, int size)
{
    ObjectGUID instanceId = 0UL;
    if (auto it = m_mapObjectCreatorFunctions.find(tid); it != m_mapObjectCreatorFunctions.end())
    {
		ObjectCreatorFunction* pCreatorFunc = it->second;
        pCreatorFunc(tid, pData, size);
        instanceId = 1UL;
        //if (Object* pObject = pCreatorFunc(tid, pData, size))
        //{
        //    instanceId = 1UL; //pObject->GetInstanceId();
        //}
    }

    return instanceId;
}

void CSharpBridge::DestroyObject(ObjectTypeGUID tid, ObjectGUID instanceId)
{

}

void CSharpBridge::SetObjectProperty(ObjectTypeGUID tid, ObjectPropertyUID pid, ObjectGUID instanceId, void* data, int size)
{
    uint64_t tpid = MakeU64(tid, pid);
    if (auto it = m_mapObjectPropertyFunctions.find(tpid); it != m_mapObjectPropertyFunctions.end())
    {
        ObjectPropertySetFunction* pSetter = it->second.first;
        assert(pSetter && "Failed to set property");
        pSetter(instanceId, data, size);
    }
}

void CSharpBridge::GetObjectProperty(ObjectTypeGUID tid, ObjectPropertyUID pid, ObjectGUID instanceId, void** ppData, int* pSize)
{
    uint64_t tpid = MakeU64(tid, pid);
    if (auto it = m_mapObjectPropertyFunctions.find(tpid); it != m_mapObjectPropertyFunctions.end())
    {
        ObjectPropertyGetFunction* pGetter = it->second.second;
        assert(pGetter && "Failed to get property");
        pGetter(instanceId, ppData, pSize);
    }
}

}