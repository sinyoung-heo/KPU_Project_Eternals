#include "stdafx.h"
#include "ObjMgr.h"

IMPLEMENT_SINGLETON(CObjMgr)

CObjMgr::CObjMgr(void)
{
}

CObjMgr::~CObjMgr(void)
{
}

HRESULT CObjMgr::Init_ObjMgr()
{
	m_mapObjList[L"PLAYER"] = OBJLIST();
	m_mapObjList[L"NPC"] = OBJLIST();

	return S_OK;
}

CObj* CObjMgr::Get_GameObject(wstring wstrObjTag, int server_num)
{
	/* map에서 찾고자 하는 OBJLIST를 Key 값을 통해 찾기 */
	auto& iter_find = m_mapObjList.find(wstrObjTag);

	/* 해당 OBJLIST를 찾지 못하였다면 NULL 반환 */
	if (iter_find == m_mapObjList.end())
		return nullptr;

	/* server number값이 있는지 탐색 */
	auto& user = iter_find->second.find(server_num);

	/* 탐색 실패 */
	if (user == iter_find->second.end())
		return nullptr;

	/* 탐색 성공 -> 객체 찾기 성공 */
	return user->second;
}

OBJLIST* CObjMgr::Get_OBJLIST(wstring wstrObjTag)
{
	/* map에서 찾고자 하는 OBJLIST를 key 값을 통해 찾기 */
	auto& iter_find = m_mapObjList.find(wstrObjTag);

	/* 해당 OBJLIST를 찾지 못하였다면 NULL 반환 */
	if (iter_find == m_mapObjList.end())
		return nullptr;

	return &(iter_find->second);
}

bool CObjMgr::Is_Player(int server_num)
{
	/* PLAYER ObjList 에서 찾고자 하는 OBJLIST를 key 값을 통해 찾기 */
	auto& iter_find = m_mapObjList[L"PLAYER"].find(server_num);

	/* 해당 OBJLIST를 찾지 못하였다면 NULL 반환 */
	if (iter_find == m_mapObjList[L"PLAYER"].end())
		return false;

	return true;
}

bool CObjMgr::Is_Near(const CObj* me, const CObj* other)
{
	float dist = (other->m_vPos.x - me->m_vPos.x) * (other->m_vPos.x - me->m_vPos.x);
	dist += (other->m_vPos.y - me->m_vPos.y) * (other->m_vPos.y - me->m_vPos.y);
	dist += (other->m_vPos.z - me->m_vPos.z) * (other->m_vPos.z - me->m_vPos.z);

	return dist <= VIEW_LIMIT * VIEW_LIMIT;
}

HRESULT CObjMgr::Add_GameObject(wstring wstrObjTag, CObj* pObj, int server_num)
{
	if (pObj != nullptr)
	{
		/* map에서 찾고자 하는 OBJLIST를 key 값을 통해 찾기 */
		auto& iter_find = m_mapObjList.find(wstrObjTag);

		/* 해당 OBJLIST가 없다면, 임시 OBJLIST 생성한 후 오브젝트 삽입 */
		if (iter_find == m_mapObjList.end())
			m_mapObjList[wstrObjTag] = OBJLIST();
		
		m_mapObjList[wstrObjTag][server_num] = pObj;
	}

	return S_OK;
}

HRESULT CObjMgr::Delete_GameObject(wstring wstrObjTag, CObj* pObj)
{
	if (pObj != nullptr)
	{
		/* map에서 찾고자 하는 OBJLIST를 key 값을 통해 찾기 */
		auto& iter_find = m_mapObjList.find(wstrObjTag);

		/* 해당 OBJLIST가 없다면, 임시 OBJLIST 생성한 후 오브젝트 삽입 */
		if (iter_find == m_mapObjList.end())
			return E_FAIL;

		/* server number 값이 있는지 탐색 */
		auto& user = iter_find->second.find(pObj->m_sNum);
		
		/* 탐색 실패 */
		if (user == iter_find->second.end())
			return E_FAIL;

		/* 3.탐색 성공 -> 삭제 */
		if (user->second)
		{
			iter_find->second.erase(pObj->m_sNum);
			return S_OK;
		}
	}

	return S_OK;
}

HRESULT CObjMgr::Delete_OBJLIST(wstring wstrObjTag)
{
	m_mapObjList[wstrObjTag].clear();
	return S_OK;
}

void CObjMgr::Release()
{
	for (auto& o_list : m_mapObjList)
	{	
		o_list.second.clear();
	}
}
