#include "stdafx.h"
#include "Monster.h"
#include "Player.h"

CMonster::CMonster()
	:m_iHp(0), m_iMaxHp(0), m_iExp(0), m_iAtt(0), m_fSpd(0.f),
	m_iTargetNum(-1), m_bIsAttack(false), m_bIsComeBack(false),
	m_monNum(0), m_bIsShortAttack(true), m_uiAnimIdx(0)
{
}

CMonster::~CMonster()
{
}

void CMonster::Set_AnimDuration(double arr[])
{
	for (int i = 0; i < MAX_ANI; ++i)
		m_arrDuration[i] = arr[i];
}


int CMonster::Update_Monster(const float& fTimeDelta)
{
	if (m_bIsDead)
		return DEAD_OBJ;

	if (fTimeDelta > 1.f)
		return NO_EVENT;

	Change_Animation(fTimeDelta);

	/* Calculate Animation frame */
	Set_AnimationKey(m_uiAnimIdx);
	Play_Animation(fTimeDelta * Monster_Normal::TPS);

	return NO_EVENT;
}

void CMonster::Change_Animation(const float& fTimeDelta)
{
	if (m_monNum == MON_CRAB)
		Change_Crab_Animation(fTimeDelta);
	else if (m_monNum == MON_MONKEY)
		Change_Monkey_Animation(fTimeDelta);
	else if (m_monNum == MON_CLODER)
		Change_Cloder_Animation(fTimeDelta);
	else if (m_monNum == MON_SAILOR)
		Change_DrownedSailor_Animation(fTimeDelta);
}

void CMonster::Change_Crab_Animation(const float& fTimeDelta)
{
	switch (m_status)
	{
		
	case STATUS::ST_ACTIVE:
	{
		Move_NormalMonster(fTimeDelta);
	}
	break;

	case STATUS::ST_NONACTIVE:
	{
		m_iTargetNum = -1;
		m_bIsComeBack = false;
	}
	break;

	case STATUS::ST_CHASE:
	{
		Chase_Crab(fTimeDelta);
	}
	break;
	
	case STATUS::ST_ATTACK:
	{
		Attack_Crab(fTimeDelta);
	}
	break;

	case STATUS::ST_DEAD:
		break;
	}
}

void CMonster::Change_Monkey_Animation(const float& fTimeDelta)
{
	switch (m_status)
	{

	case STATUS::ST_ACTIVE:
	{
		Move_NormalMonster(fTimeDelta);
	}
	break;

	case STATUS::ST_NONACTIVE:
	{
		m_iTargetNum = -1;
		m_bIsComeBack = false;
	}
	break;

	case STATUS::ST_CHASE:
	{
		Chase_Monkey(fTimeDelta);
	}
	break;

	case STATUS::ST_ATTACK:
	{
		Attack_Monkey(fTimeDelta);
	}
	break;

	case STATUS::ST_DEAD:
		break;
	}
}

void CMonster::Change_Cloder_Animation(const float& fTimeDelta)
{
	switch (m_status)
	{

	case STATUS::ST_ACTIVE:
	{
		Move_NormalMonster(fTimeDelta);
	}
	break;

	case STATUS::ST_NONACTIVE:
	{
		m_iTargetNum = -1;
		m_bIsComeBack = false;
	}
	break;

	case STATUS::ST_CHASE:
	{
		Chase_Cloder(fTimeDelta);
	}
	break;

	case STATUS::ST_ATTACK:
	{
		Attack_Cloder(fTimeDelta);
	}
	break;

	case STATUS::ST_DEAD:
		break;
	}
}

void CMonster::Change_DrownedSailor_Animation(const float& fTimeDelta)
{
	switch (m_status)
	{

	case STATUS::ST_ACTIVE:
	{
		m_uiAnimIdx = Monster_Normal::WALK;
		Move_NormalMonster(fTimeDelta);
	}
	break;

	case STATUS::ST_NONACTIVE:
	{
		m_iTargetNum = -1;
		m_bIsComeBack = false;
		m_uiAnimIdx = Monster_Normal::WAIT;
	}
	break;

	case STATUS::ST_CHASE:
	{
		m_uiAnimIdx = Monster_Normal::RUN;
		Chase_DrownedSailor(fTimeDelta);
	}
	break;

	case STATUS::ST_ATTACK:
	{
		Attack_DrownedSailor(fTimeDelta);
	}
	break;

	case STATUS::ST_DEAD:
		break;
	}
}

void CMonster::Move_ComeBack(const float& fTimeDelta)
{
	m_bIsComeBack = true;

	/* 해당 Monster의 원래 위치값 */
	float ori_x, ori_y, ori_z;
	ori_x = m_vPos.x;
	ori_y = m_vPos.y;
	ori_z = m_vPos.z;

	// 움직이기 전 위치에서의 viewlist (시야 내에 플레이어 저장)
	unordered_set<pair<int, int>> oldnearSector;
	oldnearSector.reserve(5);
	CSectorMgr::GetInstance()->Get_NearSectorIndex(&oldnearSector, (int)ori_x, (int)ori_z);

	unordered_set <int> old_viewlist;

	// 이동 전: 인접 섹터 순회
	for (auto& s : oldnearSector)
	{
		// 인접 섹터 내의 타 유저들이 있는지 검사
		if (!(CSectorMgr::GetInstance()->Get_SectorList()[s.first][s.second].Get_ObjList().empty()))
		{
			// 타 유저의 서버 번호 추출
			for (auto obj_num : CSectorMgr::GetInstance()->Get_SectorList()[s.first][s.second].Get_ObjList())
			{
				/* 타유저일 경우 처리 */
				if (obj_num == m_sNum) continue;
				if (true == CObjMgr::GetInstance()->Is_Player(obj_num))
				{
					CPlayer* pPlayer = static_cast<CPlayer*>(CObjMgr::GetInstance()->Get_GameObject(L"PLAYER", obj_num));

					// 접속한 유저만 시야 목록에 등록한다.
					if (!pPlayer->Get_IsConnected()) continue;

					// 시야 내에 있다면 시야 목록에 등록한다.
					if (CObjMgr::GetInstance()->Is_Near(this, pPlayer))
						old_viewlist.insert(obj_num);
				}
			}
		}
	}

	/* Monster 움직임 처리 */
	m_vDir = m_vOriPos - m_vPos;
	m_vDir.Normalize();

	/* monster return home position */
	if (!CCollisionMgr::GetInstance()->Is_Arrive(m_vPos, m_vOriPos))
		m_vPos += m_vDir * fTimeDelta;
	else
		nonActive_monster(m_sNum);
	
	/* NaviMesh를 벗어날 경우 움직임 X */
	if (CNaviMesh::GetInstance()->Get_CurrentPositionCellIndex(m_vPos) == -1)
		return;

	/* 변경된 좌표로 섹터 갱신 */
	CSectorMgr::GetInstance()->Compare_exchange_Sector(m_sNum, (int)ori_z, (int)ori_x, (int)(m_vPos.z), (int)(m_vPos.x));

	// 움직인 후 위치에서의 viewlist (시야 내에 플레이어 저장)
	unordered_set <int> new_viewlist;

	unordered_set<pair<int, int>> nearSectors;
	nearSectors.reserve(5);
	CSectorMgr::GetInstance()->Get_NearSectorIndex(&nearSectors, (int)(m_vPos.x), (int)(m_vPos.z));

	// 이동 후: 인접 섹터 순회 -> 유저가 있을 시 new viewlist 내에 등록
	for (auto& s : nearSectors)
	{
		// 인접 섹터 내의 타 유저들이 있는지 검사
		if (!(CSectorMgr::GetInstance()->Get_SectorList()[s.first][s.second].Get_ObjList().empty()))
		{
			// 타 유저의 서버 번호 추출
			for (auto obj_num : CSectorMgr::GetInstance()->Get_SectorList()[s.first][s.second].Get_ObjList())
			{
				/* 타유저일 경우 처리 */
				if (true == CObjMgr::GetInstance()->Is_Player(obj_num))
				{
					CPlayer* pPlayer = static_cast<CPlayer*>(CObjMgr::GetInstance()->Get_GameObject(L"PLAYER", obj_num));

					// 접속한 유저만 시야 목록에 등록한다.
					if (!pPlayer->Get_IsConnected()) continue;

					// 시야 내에 있다면 시야 목록에 등록한다.
					if (CObjMgr::GetInstance()->Is_Near(this, pPlayer))
						new_viewlist.insert(obj_num);
				}
			}
		}
	}

	// 이동 전 viewlist & 이동 후 viewlist 비교 -> 각 유저들의 시야 목록 내에 Monster 존재 여부를 결정.
	for (auto pl : old_viewlist)
	{
		// 이동 후에도 Monster 시야 목록 내에 "pl"(server number) 유저가 남아있는 경우
		if (0 < new_viewlist.count(pl))
		{
			CPlayer* pPlayer = static_cast<CPlayer*>(CObjMgr::GetInstance()->Get_GameObject(L"PLAYER", pl));
			if (pPlayer != nullptr)
			{
				/* 해당 유저의 시야 목록에 현재 Monster가 존재할 경우 */
				pPlayer->v_lock.lock();
				if (0 < pPlayer->view_list.count(m_sNum))
				{
					pPlayer->v_lock.unlock();
					/* 해당 유저에게 NPC가 움직인 후의 위치를 전송 */
					send_Monster_move_packet(pl, Monster_Normal::WALK);
				}
				/* 해당 유저의 시야 목록에 현재 Monster가 존재하지 않을 경우 */
				else
				{
					/* 해당 유저의 시야 목록에 현재 Monster 등록 */
					pPlayer->view_list.insert(m_sNum);
					pPlayer->v_lock.unlock();
					send_Monster_enter_packet(pl);
				}
			}
		}
		// 이동 후에 Monster 시야 목록 내에 "pl"(server number) 유저가 없는 경우
		else
		{
			CPlayer* pPlayer = static_cast<CPlayer*>(CObjMgr::GetInstance()->Get_GameObject(L"PLAYER", pl));
			if (pPlayer != nullptr)
			{
				/* 해당 유저의 시야 목록에 현재 Monster가 존재할 경우 */
				pPlayer->v_lock.lock();
				if (0 < pPlayer->view_list.count(m_sNum))
				{
					/* 해당 유저의 시야 목록에서 현재 Monster 삭제 */
					pPlayer->view_list.erase(m_sNum);
					pPlayer->v_lock.unlock();
					send_leave_packet(pl, m_sNum);
				}
				else
					pPlayer->v_lock.unlock();
			}
		}
	}

	// new_vielist 순회 -> 플레이어의 시야 목록에 있어야 할 새로운 Monster들을 추가
	for (auto pl : new_viewlist)
	{
		CPlayer* pPlayer = static_cast<CPlayer*>(CObjMgr::GetInstance()->Get_GameObject(L"PLAYER", pl));
		if (pPlayer != nullptr)
		{
			pPlayer->v_lock.lock();
			if (0 == pPlayer->view_list.count(pl))
			{
				/* 각 유저의 시야 목록 내에 현재 Monster가 없을 경우 -> 현재 Monster 등록 */
				if (0 == pPlayer->view_list.count(m_sNum))
				{
					pPlayer->view_list.insert(m_sNum);
					pPlayer->v_lock.unlock();
					send_Monster_enter_packet(pl);
				}
				/* 각 유저의 시야 목록 내에 현재 Monster가 있을 경우 -> 현재 Monster 위치 전송 */
				else
				{
					pPlayer->v_lock.unlock();
					send_Monster_move_packet(pl, Monster_Normal::WALK);
				}
			}
			else
				pPlayer->v_lock.unlock();
		}

	}	
}

void CMonster::Move_NormalMonster(const float& fTimeDelta)
{
	/* 해당 Monster의 원래 위치값 */
	float ori_x, ori_y, ori_z;
	ori_x = m_vPos.x;
	ori_y = m_vPos.y;
	ori_z = m_vPos.z;

	m_fSpd = 1.f;

	// 움직이기 전 위치에서의 viewlist (시야 내에 플레이어 저장)
	unordered_set<pair<int, int>> oldnearSector;
	oldnearSector.reserve(5);
	CSectorMgr::GetInstance()->Get_NearSectorIndex(&oldnearSector, (int)ori_x, (int)ori_z);

	unordered_set <int> old_viewlist;

	// 이동 전: 인접 섹터 순회
	for (auto& s : oldnearSector)
	{
		// 인접 섹터 내의 타 유저들이 있는지 검사
		if (!(CSectorMgr::GetInstance()->Get_SectorList()[s.first][s.second].Get_ObjList().empty()))
		{
			// 타 유저의 서버 번호 추출
			for (auto obj_num : CSectorMgr::GetInstance()->Get_SectorList()[s.first][s.second].Get_ObjList())
			{
				/* 타유저일 경우 처리 */
				if (obj_num == m_sNum) continue;
				if (true == CObjMgr::GetInstance()->Is_Player(obj_num))
				{
					CPlayer* pPlayer = static_cast<CPlayer*>(CObjMgr::GetInstance()->Get_GameObject(L"PLAYER", obj_num));

					// 접속한 유저만 시야 목록에 등록한다.
					if (!pPlayer->Get_IsConnected()) continue;

					// 시야 내에 있다면 시야 목록에 등록한다.
					if (CObjMgr::GetInstance()->Is_Near(this, pPlayer))
						old_viewlist.insert(obj_num);
				}
			}
		}
	}

	/* Monster 움직임 방향 처리 */
	if (CCollisionMgr::GetInstance()->Is_Arrive(m_vPos, m_vTempPos))
	{
		switch (rand() % 8)
		{
		case 0: m_vDir = _vec3(0.f, 0.f, 1.f); break;
		case 1: m_vDir = _vec3(0.f, 0.f, -1.f); break;
		case 2: m_vDir = _vec3(1.f, 0.f, 0.f); break;
		case 3: m_vDir = _vec3(1.f, 0.f, 1.f); break;
		case 4: m_vDir = _vec3(1.f, 0.f, -1.f); break;
		case 5: m_vDir = _vec3(-1.f, 0.f, 0.f); break;
		case 6: m_vDir = _vec3(-1.f, 0.f, 1.f); break;
		case 7: m_vDir = _vec3(-1.f, 0.f, -1.f); break;
		}

		/* 해당 NPC의 미래 위치 좌표 산출 -> 미래 위치좌표는 임시 변수에 저장 */
		m_vTempPos += m_vDir * 5.f;

		if (false == CCollisionMgr::GetInstance()->Is_InMoveLimit(m_vTempPos, m_vOriPos))
		{
			m_vTempPos = m_vPos;
			return;
		}
	}

	/* Monster Move */
	m_vPos += m_vDir * m_fSpd * fTimeDelta;

	/* NaviMesh를 벗어날 경우 움직임 X */
	if (CNaviMesh::GetInstance()->Get_CurrentPositionCellIndex(m_vPos) == -1)
		return;

	/* 변경된 좌표로 섹터 갱신 */
	CSectorMgr::GetInstance()->Compare_exchange_Sector(m_sNum, (int)ori_z, (int)ori_x, (int)(m_vPos.z), (int)(m_vPos.x));

	// 움직인 후 위치에서의 viewlist (시야 내에 플레이어 저장)
	unordered_set <int> new_viewlist;

	unordered_set<pair<int, int>> nearSectors;
	nearSectors.reserve(5);
	CSectorMgr::GetInstance()->Get_NearSectorIndex(&nearSectors, (int)(m_vPos.x), (int)(m_vPos.z));

	// 이동 후: 인접 섹터 순회 -> 유저가 있을 시 new viewlist 내에 등록
	for (auto& s : nearSectors)
	{
		// 인접 섹터 내의 타 유저들이 있는지 검사
		if (!(CSectorMgr::GetInstance()->Get_SectorList()[s.first][s.second].Get_ObjList().empty()))
		{
			// 타 유저의 서버 번호 추출
			for (auto obj_num : CSectorMgr::GetInstance()->Get_SectorList()[s.first][s.second].Get_ObjList())
			{
				/* 타유저일 경우 처리 */
				if (true == CObjMgr::GetInstance()->Is_Player(obj_num))
				{
					CPlayer* pPlayer = static_cast<CPlayer*>(CObjMgr::GetInstance()->Get_GameObject(L"PLAYER", obj_num));

					// 접속한 유저만 시야 목록에 등록한다.
					if (!pPlayer->Get_IsConnected()) continue;

					// 시야 내에 있다면 시야 목록에 등록한다.
					if (CObjMgr::GetInstance()->Is_Near(this, pPlayer))
						new_viewlist.insert(obj_num);
				}
			}
		}
	}

	// 이동 전 viewlist & 이동 후 viewlist 비교 -> 각 유저들의 시야 목록 내에 Monster 존재 여부를 결정.
	for (auto pl : old_viewlist)
	{
		// 이동 후에도 Monster 시야 목록 내에 "pl"(server number) 유저가 남아있는 경우
		if (0 < new_viewlist.count(pl))
		{
			CPlayer* pPlayer = static_cast<CPlayer*>(CObjMgr::GetInstance()->Get_GameObject(L"PLAYER", pl));
			if (pPlayer != nullptr)
			{
				/* 해당 유저의 시야 목록에 현재 Monster가 존재할 경우 */
				pPlayer->v_lock.lock();
				if (0 < pPlayer->view_list.count(m_sNum))
				{
					pPlayer->v_lock.unlock();
					/* 해당 유저에게 NPC가 움직인 후의 위치를 전송 */
					send_Monster_move_packet(pl, Monster_Normal::WALK);
				}
				/* 해당 유저의 시야 목록에 현재 Monster가 존재하지 않을 경우 */
				else
				{
					/* 해당 유저의 시야 목록에 현재 Monster 등록 */
					pPlayer->view_list.insert(m_sNum);
					pPlayer->v_lock.unlock();
					send_Monster_enter_packet(pl);
				}
			}
		}
		// 이동 후에 Monster 시야 목록 내에 "pl"(server number) 유저가 없는 경우
		else
		{
			CPlayer* pPlayer = static_cast<CPlayer*>(CObjMgr::GetInstance()->Get_GameObject(L"PLAYER", pl));
			if (pPlayer != nullptr)
			{
				/* 해당 유저의 시야 목록에 현재 Monster가 존재할 경우 */
				pPlayer->v_lock.lock();
				if (0 < pPlayer->view_list.count(m_sNum))
				{
					/* 해당 유저의 시야 목록에서 현재 Monster 삭제 */
					pPlayer->view_list.erase(m_sNum);
					pPlayer->v_lock.unlock();
					send_leave_packet(pl, m_sNum);
				}
				else
					pPlayer->v_lock.unlock();
			}
		}
	}

	// new_vielist 순회 -> 플레이어의 시야 목록에 있어야 할 새로운 Monster들을 추가
	for (auto pl : new_viewlist)
	{
		CPlayer* pPlayer = static_cast<CPlayer*>(CObjMgr::GetInstance()->Get_GameObject(L"PLAYER", pl));
		if (pPlayer != nullptr)
		{
			pPlayer->v_lock.lock();
			if (0 == pPlayer->view_list.count(pl))
			{
				/* 각 유저의 시야 목록 내에 현재 Monster가 없을 경우 -> 현재 Monster 등록 */
				if (0 == pPlayer->view_list.count(m_sNum))
				{
					pPlayer->view_list.insert(m_sNum);
					pPlayer->v_lock.unlock();
					send_Monster_enter_packet(pl);
				}
				/* 각 유저의 시야 목록 내에 현재 Monster가 있을 경우 -> 현재 Monster 위치 전송 */
				else
				{
					pPlayer->v_lock.unlock();
					send_Monster_move_packet(pl, Monster_Normal::WALK);
				}
			}
			else
				pPlayer->v_lock.unlock();
		}

	}

	// Monster 시야 내에 아무도 없다면 NON ACTIVE로 상태 변경
	if (new_viewlist.empty() == true)
		nonActive_monster(m_sNum);
}

void CMonster::Chase_Crab(const float& fTimeDelta)
{
	/* 해당 Monster의 원래 위치값 */
	float ori_x, ori_y, ori_z;
	ori_x = m_vPos.x;
	ori_y = m_vPos.y;
	ori_z = m_vPos.z;

	m_fSpd = 2.0f;

	// 움직이기 전 위치에서의 viewlist (시야 내에 플레이어 저장)
	unordered_set<pair<int, int>> oldnearSector;
	oldnearSector.reserve(5);
	CSectorMgr::GetInstance()->Get_NearSectorIndex(&oldnearSector, (int)ori_x, (int)ori_z);

	unordered_set <int> old_viewlist;
	unordered_set <int> old_targetList;

	// 이동 전: 인접 섹터 순회 (몬스터 시야 파악)
	for (auto& s : oldnearSector)
	{
		// 인접 섹터 내의 타 유저들이 있는지 검사
		if (!(CSectorMgr::GetInstance()->Get_SectorList()[s.first][s.second].Get_ObjList().empty()))
		{
			// 타 유저의 서버 번호 추출
			for (auto obj_num : CSectorMgr::GetInstance()->Get_SectorList()[s.first][s.second].Get_ObjList())
			{
				/* 타유저일 경우 처리 */
				if (true == CObjMgr::GetInstance()->Is_Player(obj_num))
				{
					CPlayer* pPlayer = static_cast<CPlayer*>(CObjMgr::GetInstance()->Get_GameObject(L"PLAYER", obj_num));

					// 접속한 유저만 시야 목록에 등록한다.
					if (!pPlayer->Get_IsConnected()) continue;

					// 시야 내에 있다면 시야 목록에 등록한다.
					if (CObjMgr::GetInstance()->Is_Near(this, pPlayer))
						old_viewlist.insert(obj_num);

					// 몬스터 추적 범위 내에 있는 유저 탐색한다.
					if (CObjMgr::GetInstance()->Is_Monster_Target(this, pPlayer))
						old_targetList.insert(obj_num);
				}
			}
		}
	}

	/* 타겟(공격 대상)이 존재할 경우 -> 타겟 추적 */
	if (!old_targetList.empty())
	{
		int target_id = *(old_targetList.begin());

		CPlayer* pTarget = static_cast<CPlayer*>(CObjMgr::GetInstance()->Get_GameObject(L"PLAYER", target_id));
		if (pTarget == nullptr) 		
			return;
		
		/* 타겟을 추적하기 위한 방향으로 설정 */
		m_vDir = pTarget->m_vPos - m_vPos;
		m_vDir.Normalize();

		/* monster chase move -> arrive at player -> start to attack */
		if (!CCollisionMgr::GetInstance()->Is_Arrive(m_vPos, pTarget->m_vPos))
			m_vPos += m_vDir * m_fSpd * fTimeDelta;
		else
			Change_AttackMode();
	}
	/* 타겟(공격 대상)이 존재하지 않을 경우 -> 생성된 위치로 돌아감 */
	else
	{
		m_vDir = m_vOriPos - m_vPos;
		m_vDir.Normalize();

		/* monster return home position */
		if (!CCollisionMgr::GetInstance()->Is_Arrive(m_vPos, m_vOriPos))
			m_vPos += m_vDir * fTimeDelta;
		else
			nonActive_monster(m_sNum);
	}

	/* NaviMesh를 벗어날 경우 움직임 X */
	if (CNaviMesh::GetInstance()->Get_CurrentPositionCellIndex(m_vPos) == -1)
		return;
	
	/* 변경된 좌표로 섹터 갱신 */
	CSectorMgr::GetInstance()->Compare_exchange_Sector(m_sNum, (int)ori_z, (int)ori_x, (int)(m_vPos.z), (int)(m_vPos.x));

	// 움직인 후 위치에서의 viewlist (시야 내에 플레이어 저장)
	unordered_set <int> new_viewlist;

	unordered_set<pair<int, int>> nearSectors;
	nearSectors.reserve(5);
	CSectorMgr::GetInstance()->Get_NearSectorIndex(&nearSectors, (int)(m_vPos.x), (int)(m_vPos.z));

	// 이동 후: 인접 섹터 순회 -> 유저가 있을 시 new viewlist 내에 등록
	for (auto& s : nearSectors)
	{
		// 인접 섹터 내의 타 유저들이 있는지 검사
		if (!(CSectorMgr::GetInstance()->Get_SectorList()[s.first][s.second].Get_ObjList().empty()))
		{
			// 타 유저의 서버 번호 추출
			for (auto obj_num : CSectorMgr::GetInstance()->Get_SectorList()[s.first][s.second].Get_ObjList())
			{
				/* 타유저일 경우 처리 */
				if (true == CObjMgr::GetInstance()->Is_Player(obj_num))
				{
					CPlayer* pPlayer = static_cast<CPlayer*>(CObjMgr::GetInstance()->Get_GameObject(L"PLAYER", obj_num));

					// 접속한 유저만 시야 목록에 등록한다.
					if (!pPlayer->Get_IsConnected()) continue;

					// 시야 내에 있다면 시야 목록에 등록한다.
					if (CObjMgr::GetInstance()->Is_Near(this, pPlayer))
						new_viewlist.insert(obj_num);
				}
			}
		}
	}

	// 이동 전 viewlist & 이동 후 viewlist 비교 -> 각 유저들의 시야 목록 내에 Monster 존재 여부를 결정.
	for (auto pl : old_viewlist)
	{
		// 이동 후에도 Monster 시야 목록 내에 "pl"(server number) 유저가 남아있는 경우
		if (0 < new_viewlist.count(pl))
		{
			CPlayer* pPlayer = static_cast<CPlayer*>(CObjMgr::GetInstance()->Get_GameObject(L"PLAYER", pl));
			if (pPlayer != nullptr)
			{
				/* 해당 유저의 시야 목록에 현재 Monster가 존재할 경우 */
				pPlayer->v_lock.lock();
				if (0 < pPlayer->view_list.count(m_sNum))
				{
					pPlayer->v_lock.unlock();
					/* 해당 유저에게 NPC가 움직인 후의 위치를 전송 */
					send_Monster_move_packet(pl, Monster_Normal::RUN);
				}
				/* 해당 유저의 시야 목록에 현재 Monster가 존재하지 않을 경우 */
				else
				{
					/* 해당 유저의 시야 목록에 현재 Monster 등록 */
					pPlayer->view_list.insert(m_sNum);
					pPlayer->v_lock.unlock();
					send_Monster_enter_packet(pl);
				}
			}
		}
		// 이동 후에 Monster 시야 목록 내에 "pl"(server number) 유저가 없는 경우
		else
		{
			CPlayer* pPlayer = static_cast<CPlayer*>(CObjMgr::GetInstance()->Get_GameObject(L"PLAYER", pl));
			if (pPlayer != nullptr)
			{
				/* 해당 유저의 시야 목록에 현재 Monster가 존재할 경우 */
				pPlayer->v_lock.lock();
				if (0 < pPlayer->view_list.count(m_sNum))
				{
					/* 해당 유저의 시야 목록에서 현재 Monster 삭제 */
					pPlayer->view_list.erase(m_sNum);
					pPlayer->v_lock.unlock();
					send_leave_packet(pl, m_sNum);
				}
				else
					pPlayer->v_lock.unlock();
			}
		}
	}

	// new_vielist 순회 -> 플레이어의 시야 목록에 있어야 할 새로운 Monster들을 추가
	for (auto pl : new_viewlist)
	{
		CPlayer* pPlayer = static_cast<CPlayer*>(CObjMgr::GetInstance()->Get_GameObject(L"PLAYER", pl));
		if (pPlayer != nullptr)
		{
			pPlayer->v_lock.lock();
			if (0 == pPlayer->view_list.count(pl))
			{
				/* 각 유저의 시야 목록 내에 현재 Monster가 없을 경우 -> 현재 Monster 등록 */
				if (0 == pPlayer->view_list.count(m_sNum))
				{
					pPlayer->view_list.insert(m_sNum);
					pPlayer->v_lock.unlock();
					send_Monster_enter_packet(pl);
				}
				/* 각 유저의 시야 목록 내에 현재 Monster가 있을 경우 -> 현재 Monster 위치 전송 */
				else
				{
					pPlayer->v_lock.unlock();
					send_Monster_move_packet(pl, Monster_Normal::RUN);
				}
			}
			else
				pPlayer->v_lock.unlock();
		}
	}
}

void CMonster::Chase_Monkey(const float& fTimeDelta)
{
	/* 해당 Monster의 원래 위치값 */
	float ori_x, ori_y, ori_z;
	ori_x = m_vPos.x;
	ori_y = m_vPos.y;
	ori_z = m_vPos.z;

	m_fSpd = 3.0f;

	// 움직이기 전 위치에서의 viewlist (시야 내에 플레이어 저장)
	unordered_set<pair<int, int>> oldnearSector;
	oldnearSector.reserve(5);
	CSectorMgr::GetInstance()->Get_NearSectorIndex(&oldnearSector, (int)ori_x, (int)ori_z);

	unordered_set <int> old_viewlist;
	unordered_set <int> old_targetList;

	// 이동 전: 인접 섹터 순회 (몬스터 시야 파악)
	for (auto& s : oldnearSector)
	{
		// 인접 섹터 내의 타 유저들이 있는지 검사
		if (!(CSectorMgr::GetInstance()->Get_SectorList()[s.first][s.second].Get_ObjList().empty()))
		{
			// 타 유저의 서버 번호 추출
			for (auto obj_num : CSectorMgr::GetInstance()->Get_SectorList()[s.first][s.second].Get_ObjList())
			{
				/* 타유저일 경우 처리 */
				if (true == CObjMgr::GetInstance()->Is_Player(obj_num))
				{
					CPlayer* pPlayer = static_cast<CPlayer*>(CObjMgr::GetInstance()->Get_GameObject(L"PLAYER", obj_num));

					// 접속한 유저만 시야 목록에 등록한다.
					if (!pPlayer->Get_IsConnected()) continue;

					// 시야 내에 있다면 시야 목록에 등록한다.
					if (CObjMgr::GetInstance()->Is_Near(this, pPlayer))
						old_viewlist.insert(obj_num);

					// 몬스터 추적 범위 내에 있는 유저 탐색한다.
					if (CObjMgr::GetInstance()->Is_Monster_AttackTarget(this, pPlayer, THROW_RANGE_MONKEY_END))
						old_targetList.insert(obj_num);
				}
			}
		}
	}

	/* 타겟(추적 대상)이 존재할 경우 -> 타겟 추적 */
	if (!old_targetList.empty())
	{
		if (m_iTargetNum < 0)
			m_iTargetNum = *(old_targetList.begin());
	
		CPlayer* pTarget = static_cast<CPlayer*>(CObjMgr::GetInstance()->Get_GameObject(L"PLAYER", m_iTargetNum));
		if (pTarget == nullptr)
			return;

		/* 타겟을 추적하기 위한 방향으로 설정 */
		m_vDir = pTarget->m_vPos - m_vPos;
		m_vDir.Normalize();

		/* monster chase move -> arrive at player -> start to attack */

		/* 원거리 & 근거리 공격 타입 설정 */
		float fDist = Calculate_TargetDist(pTarget->m_vPos);
		if ((THROW_RANGE_MONKEY_START * THROW_RANGE_MONKEY_START) < fDist && fDist < (THROW_RANGE_MONKEY_END * THROW_RANGE_MONKEY_END))
		{
			m_bIsShortAttack = false;
			Change_AttackMode();
		}

		else if ((ATTACK_RANGE_MONKEY * ATTACK_RANGE_MONKEY) < fDist)
			m_vPos += m_vDir * m_fSpd * fTimeDelta;
		else if ((ATTACK_RANGE_MONKEY * ATTACK_RANGE_MONKEY) >= fDist)
		{
			m_bIsShortAttack = true;
			Change_AttackMode();
		}
	}
	/* 타겟(공격 대상)이 존재하지 않을 경우 -> 생성된 위치로 돌아감 */
	else
	{
		m_vDir = m_vOriPos - m_vPos;
		m_vDir.Normalize();

		m_iTargetNum = -1;

		/* monster return home position */
		if (!CCollisionMgr::GetInstance()->Is_Arrive(m_vPos, m_vOriPos))
			m_vPos += m_vDir * fTimeDelta;
		else
			nonActive_monster(m_sNum);
	}

	/* NaviMesh를 벗어날 경우 움직임 X */
	if (CNaviMesh::GetInstance()->Get_CurrentPositionCellIndex(m_vPos) == -1)
		return;

	/* 변경된 좌표로 섹터 갱신 */
	CSectorMgr::GetInstance()->Compare_exchange_Sector(m_sNum, (int)ori_z, (int)ori_x, (int)(m_vPos.z), (int)(m_vPos.x));

	// 움직인 후 위치에서의 viewlist (시야 내에 플레이어 저장)
	unordered_set <int> new_viewlist;

	unordered_set<pair<int, int>> nearSectors;
	nearSectors.reserve(5);
	CSectorMgr::GetInstance()->Get_NearSectorIndex(&nearSectors, (int)(m_vPos.x), (int)(m_vPos.z));

	// 이동 후: 인접 섹터 순회 -> 유저가 있을 시 new viewlist 내에 등록
	for (auto& s : nearSectors)
	{
		// 인접 섹터 내의 타 유저들이 있는지 검사
		if (!(CSectorMgr::GetInstance()->Get_SectorList()[s.first][s.second].Get_ObjList().empty()))
		{
			// 타 유저의 서버 번호 추출
			for (auto obj_num : CSectorMgr::GetInstance()->Get_SectorList()[s.first][s.second].Get_ObjList())
			{
				/* 타유저일 경우 처리 */
				if (true == CObjMgr::GetInstance()->Is_Player(obj_num))
				{
					CPlayer* pPlayer = static_cast<CPlayer*>(CObjMgr::GetInstance()->Get_GameObject(L"PLAYER", obj_num));

					// 접속한 유저만 시야 목록에 등록한다.
					if (!pPlayer->Get_IsConnected()) continue;

					// 시야 내에 있다면 시야 목록에 등록한다.
					if (CObjMgr::GetInstance()->Is_Near(this, pPlayer))
						new_viewlist.insert(obj_num);
				}
			}
		}
	}

	// 이동 전 viewlist & 이동 후 viewlist 비교 -> 각 유저들의 시야 목록 내에 Monster 존재 여부를 결정.
	for (auto pl : old_viewlist)
	{
		// 이동 후에도 Monster 시야 목록 내에 "pl"(server number) 유저가 남아있는 경우
		if (0 < new_viewlist.count(pl))
		{
			CPlayer* pPlayer = static_cast<CPlayer*>(CObjMgr::GetInstance()->Get_GameObject(L"PLAYER", pl));
			if (pPlayer != nullptr)
			{
				/* 해당 유저의 시야 목록에 현재 Monster가 존재할 경우 */
				pPlayer->v_lock.lock();
				if (0 < pPlayer->view_list.count(m_sNum))
				{
					pPlayer->v_lock.unlock();
					/* 해당 유저에게 NPC가 움직인 후의 위치를 전송 */
					send_Monster_move_packet(pl, Monster_Normal::RUN);
				}
				/* 해당 유저의 시야 목록에 현재 Monster가 존재하지 않을 경우 */
				else
				{
					/* 해당 유저의 시야 목록에 현재 Monster 등록 */
					pPlayer->view_list.insert(m_sNum);
					pPlayer->v_lock.unlock();
					send_Monster_enter_packet(pl);
				}
			}
		}
		// 이동 후에 Monster 시야 목록 내에 "pl"(server number) 유저가 없는 경우
		else
		{
			CPlayer* pPlayer = static_cast<CPlayer*>(CObjMgr::GetInstance()->Get_GameObject(L"PLAYER", pl));
			if (pPlayer != nullptr)
			{
				/* 해당 유저의 시야 목록에 현재 Monster가 존재할 경우 */
				pPlayer->v_lock.lock();
				if (0 < pPlayer->view_list.count(m_sNum))
				{
					/* 해당 유저의 시야 목록에서 현재 Monster 삭제 */
					pPlayer->view_list.erase(m_sNum);
					pPlayer->v_lock.unlock();
					send_leave_packet(pl, m_sNum);
				}
				else
					pPlayer->v_lock.unlock();
			}
		}
	}

	// new_vielist 순회 -> 플레이어의 시야 목록에 있어야 할 새로운 Monster들을 추가
	for (auto pl : new_viewlist)
	{
		CPlayer* pPlayer = static_cast<CPlayer*>(CObjMgr::GetInstance()->Get_GameObject(L"PLAYER", pl));
		if (pPlayer != nullptr)
		{
			pPlayer->v_lock.lock();
			if (0 == pPlayer->view_list.count(pl))
			{
				/* 각 유저의 시야 목록 내에 현재 Monster가 없을 경우 -> 현재 Monster 등록 */
				if (0 == pPlayer->view_list.count(m_sNum))
				{
					pPlayer->view_list.insert(m_sNum);
					pPlayer->v_lock.unlock();
					send_Monster_enter_packet(pl);
				}
				/* 각 유저의 시야 목록 내에 현재 Monster가 있을 경우 -> 현재 Monster 위치 전송 */
				else
				{
					pPlayer->v_lock.unlock();
					send_Monster_move_packet(pl, Monster_Normal::RUN);
				}
			}
			else
				pPlayer->v_lock.unlock();
		}
	}
}

void CMonster::Chase_Cloder(const float& fTimeDelta)
{
	/* 해당 Monster의 원래 위치값 */
	float ori_x, ori_y, ori_z;
	ori_x = m_vPos.x;
	ori_y = m_vPos.y;
	ori_z = m_vPos.z;

	m_fSpd = 4.0f;

	// 움직이기 전 위치에서의 viewlist (시야 내에 플레이어 저장)
	unordered_set<pair<int, int>> oldnearSector;
	oldnearSector.reserve(5);
	CSectorMgr::GetInstance()->Get_NearSectorIndex(&oldnearSector, (int)ori_x, (int)ori_z);

	unordered_set <int> old_viewlist;
	unordered_set <int> old_targetList;

	// 이동 전: 인접 섹터 순회 (몬스터 시야 파악)
	for (auto& s : oldnearSector)
	{
		// 인접 섹터 내의 타 유저들이 있는지 검사
		if (!(CSectorMgr::GetInstance()->Get_SectorList()[s.first][s.second].Get_ObjList().empty()))
		{
			// 타 유저의 서버 번호 추출
			for (auto obj_num : CSectorMgr::GetInstance()->Get_SectorList()[s.first][s.second].Get_ObjList())
			{
				/* 타유저일 경우 처리 */
				if (true == CObjMgr::GetInstance()->Is_Player(obj_num))
				{
					CPlayer* pPlayer = static_cast<CPlayer*>(CObjMgr::GetInstance()->Get_GameObject(L"PLAYER", obj_num));

					// 접속한 유저만 시야 목록에 등록한다.
					if (!pPlayer->Get_IsConnected()) continue;

					// 시야 내에 있다면 시야 목록에 등록한다.
					if (CObjMgr::GetInstance()->Is_Near(this, pPlayer))
						old_viewlist.insert(obj_num);

					// 몬스터 추적 범위 내에 있는 유저 탐색한다.
					if (CObjMgr::GetInstance()->Is_Monster_Target(this, pPlayer))
						old_targetList.insert(obj_num);
				}
			}
		}
	}

	/* 타겟(추적 대상)이 존재할 경우 -> 타겟 추적 */
	if (!old_targetList.empty())
	{
		if (m_iTargetNum < 0)
			m_iTargetNum = *(old_targetList.begin());

		CPlayer* pTarget = static_cast<CPlayer*>(CObjMgr::GetInstance()->Get_GameObject(L"PLAYER", m_iTargetNum));
		if (pTarget == nullptr)
			return;

		/* 타겟을 추적하기 위한 방향으로 설정 */
		m_vDir = pTarget->m_vPos - m_vPos;
		m_vDir.Normalize();

		/* monster chase move -> arrive at player -> start to attack */
		float fDist = Calculate_TargetDist(pTarget->m_vPos);
		if ((ATTACK_RANGE_EPIC * ATTACK_RANGE_EPIC) < fDist)
			m_vPos += m_vDir * m_fSpd * fTimeDelta;
		else
			Change_AttackMode();
	}
	/* 타겟(공격 대상)이 존재하지 않을 경우 -> 생성된 위치로 돌아감 */
	else
	{
		m_vDir = m_vOriPos - m_vPos;
		m_vDir.Normalize();

		m_iTargetNum = -1;

		/* monster return home position */
		if (!CCollisionMgr::GetInstance()->Is_Arrive(m_vPos, m_vOriPos))
			m_vPos += m_vDir * fTimeDelta;
		else
			nonActive_monster(m_sNum);
	}

	/* NaviMesh를 벗어날 경우 움직임 X */
	if (CNaviMesh::GetInstance()->Get_CurrentPositionCellIndex(m_vPos) == -1)
		return;

	/* 변경된 좌표로 섹터 갱신 */
	CSectorMgr::GetInstance()->Compare_exchange_Sector(m_sNum, (int)ori_z, (int)ori_x, (int)(m_vPos.z), (int)(m_vPos.x));

	// 움직인 후 위치에서의 viewlist (시야 내에 플레이어 저장)
	unordered_set <int> new_viewlist;

	unordered_set<pair<int, int>> nearSectors;
	nearSectors.reserve(5);
	CSectorMgr::GetInstance()->Get_NearSectorIndex(&nearSectors, (int)(m_vPos.x), (int)(m_vPos.z));

	// 이동 후: 인접 섹터 순회 -> 유저가 있을 시 new viewlist 내에 등록
	for (auto& s : nearSectors)
	{
		// 인접 섹터 내의 타 유저들이 있는지 검사
		if (!(CSectorMgr::GetInstance()->Get_SectorList()[s.first][s.second].Get_ObjList().empty()))
		{
			// 타 유저의 서버 번호 추출
			for (auto obj_num : CSectorMgr::GetInstance()->Get_SectorList()[s.first][s.second].Get_ObjList())
			{
				/* 타유저일 경우 처리 */
				if (true == CObjMgr::GetInstance()->Is_Player(obj_num))
				{
					CPlayer* pPlayer = static_cast<CPlayer*>(CObjMgr::GetInstance()->Get_GameObject(L"PLAYER", obj_num));

					// 접속한 유저만 시야 목록에 등록한다.
					if (!pPlayer->Get_IsConnected()) continue;

					// 시야 내에 있다면 시야 목록에 등록한다.
					if (CObjMgr::GetInstance()->Is_Near(this, pPlayer))
						new_viewlist.insert(obj_num);
				}
			}
		}
	}

	// 이동 전 viewlist & 이동 후 viewlist 비교 -> 각 유저들의 시야 목록 내에 Monster 존재 여부를 결정.
	for (auto pl : old_viewlist)
	{
		// 이동 후에도 Monster 시야 목록 내에 "pl"(server number) 유저가 남아있는 경우
		if (0 < new_viewlist.count(pl))
		{
			CPlayer* pPlayer = static_cast<CPlayer*>(CObjMgr::GetInstance()->Get_GameObject(L"PLAYER", pl));
			if (pPlayer != nullptr)
			{
				/* 해당 유저의 시야 목록에 현재 Monster가 존재할 경우 */
				pPlayer->v_lock.lock();
				if (0 < pPlayer->view_list.count(m_sNum))
				{
					pPlayer->v_lock.unlock();
					/* 해당 유저에게 NPC가 움직인 후의 위치를 전송 */
					send_Monster_move_packet(pl, Monster_Normal::RUN);
				}
				/* 해당 유저의 시야 목록에 현재 Monster가 존재하지 않을 경우 */
				else
				{
					/* 해당 유저의 시야 목록에 현재 Monster 등록 */
					pPlayer->view_list.insert(m_sNum);
					pPlayer->v_lock.unlock();
					send_Monster_enter_packet(pl);
				}
			}
		}
		// 이동 후에 Monster 시야 목록 내에 "pl"(server number) 유저가 없는 경우
		else
		{
			CPlayer* pPlayer = static_cast<CPlayer*>(CObjMgr::GetInstance()->Get_GameObject(L"PLAYER", pl));
			if (pPlayer != nullptr)
			{
				/* 해당 유저의 시야 목록에 현재 Monster가 존재할 경우 */
				pPlayer->v_lock.lock();
				if (0 < pPlayer->view_list.count(m_sNum))
				{
					/* 해당 유저의 시야 목록에서 현재 Monster 삭제 */
					pPlayer->view_list.erase(m_sNum);
					pPlayer->v_lock.unlock();
					send_leave_packet(pl, m_sNum);
				}
				else
					pPlayer->v_lock.unlock();
			}
		}
	}

	// new_vielist 순회 -> 플레이어의 시야 목록에 있어야 할 새로운 Monster들을 추가
	for (auto pl : new_viewlist)
	{
		CPlayer* pPlayer = static_cast<CPlayer*>(CObjMgr::GetInstance()->Get_GameObject(L"PLAYER", pl));
		if (pPlayer != nullptr)
		{
			pPlayer->v_lock.lock();
			if (0 == pPlayer->view_list.count(pl))
			{
				/* 각 유저의 시야 목록 내에 현재 Monster가 없을 경우 -> 현재 Monster 등록 */
				if (0 == pPlayer->view_list.count(m_sNum))
				{
					pPlayer->view_list.insert(m_sNum);
					pPlayer->v_lock.unlock();
					send_Monster_enter_packet(pl);
				}
				/* 각 유저의 시야 목록 내에 현재 Monster가 있을 경우 -> 현재 Monster 위치 전송 */
				else
				{
					pPlayer->v_lock.unlock();
					send_Monster_move_packet(pl, Monster_Normal::RUN);
				}
			}
			else
				pPlayer->v_lock.unlock();
		}
	}
}

void CMonster::Chase_DrownedSailor(const float& fTimeDelta)
{
	/* 해당 Monster의 원래 위치값 */
	float ori_x, ori_y, ori_z;
	ori_x = m_vPos.x;
	ori_y = m_vPos.y;
	ori_z = m_vPos.z;

	m_fSpd = 4.0f;

	// 움직이기 전 위치에서의 viewlist (시야 내에 플레이어 저장)
	unordered_set<pair<int, int>> oldnearSector;
	oldnearSector.reserve(5);
	CSectorMgr::GetInstance()->Get_NearSectorIndex(&oldnearSector, (int)ori_x, (int)ori_z);

	unordered_set <int> old_viewlist;
	unordered_set <int> old_targetList;

	// 이동 전: 인접 섹터 순회 (몬스터 시야 파악)
	for (auto& s : oldnearSector)
	{
		// 인접 섹터 내의 타 유저들이 있는지 검사
		if (!(CSectorMgr::GetInstance()->Get_SectorList()[s.first][s.second].Get_ObjList().empty()))
		{
			// 타 유저의 서버 번호 추출
			for (auto obj_num : CSectorMgr::GetInstance()->Get_SectorList()[s.first][s.second].Get_ObjList())
			{
				/* 타유저일 경우 처리 */
				if (true == CObjMgr::GetInstance()->Is_Player(obj_num))
				{
					CPlayer* pPlayer = static_cast<CPlayer*>(CObjMgr::GetInstance()->Get_GameObject(L"PLAYER", obj_num));

					// 접속한 유저만 시야 목록에 등록한다.
					if (!pPlayer->Get_IsConnected()) continue;

					// 시야 내에 있다면 시야 목록에 등록한다.
					if (CObjMgr::GetInstance()->Is_Near(this, pPlayer))
						old_viewlist.insert(obj_num);

					// 몬스터 추적 범위 내에 있는 유저 탐색한다.
					if (CObjMgr::GetInstance()->Is_Monster_Target(this, pPlayer))
						old_targetList.insert(obj_num);
				}
			}
		}
	}

	/* 타겟(추적 대상)이 존재할 경우 -> 타겟 추적 */
	if (!old_targetList.empty())
	{
		if (m_iTargetNum < 0)
			m_iTargetNum = *(old_targetList.begin());

		CPlayer* pTarget = static_cast<CPlayer*>(CObjMgr::GetInstance()->Get_GameObject(L"PLAYER", m_iTargetNum));
		if (pTarget == nullptr)
			return;

		/* 타겟을 추적하기 위한 방향으로 설정 */
		m_vDir = pTarget->m_vPos - m_vPos;
		m_vDir.Normalize();

		/* monster chase move -> arrive at player -> start to attack */
		float fDist = Calculate_TargetDist(pTarget->m_vPos);
		if ((ATTACK_RANGE_CRAB * ATTACK_RANGE_CRAB) < fDist)
			m_vPos += m_vDir * m_fSpd * fTimeDelta;
		else
			Change_AttackMode();
	}
	/* 타겟(공격 대상)이 존재하지 않을 경우 -> 생성된 위치로 돌아감 */
	else
	{
		m_vDir = m_vOriPos - m_vPos;
		m_vDir.Normalize();

		m_iTargetNum = -1;

		/* monster return home position */
		if (!CCollisionMgr::GetInstance()->Is_Arrive(m_vPos, m_vOriPos))
			m_vPos += m_vDir * fTimeDelta;
		else
			nonActive_monster(m_sNum);
	}

	/* NaviMesh를 벗어날 경우 움직임 X */
	if (CNaviMesh::GetInstance()->Get_CurrentPositionCellIndex(m_vPos) == -1)
		return;

	/* 변경된 좌표로 섹터 갱신 */
	CSectorMgr::GetInstance()->Compare_exchange_Sector(m_sNum, (int)ori_z, (int)ori_x, (int)(m_vPos.z), (int)(m_vPos.x));

	// 움직인 후 위치에서의 viewlist (시야 내에 플레이어 저장)
	unordered_set <int> new_viewlist;

	unordered_set<pair<int, int>> nearSectors;
	nearSectors.reserve(5);
	CSectorMgr::GetInstance()->Get_NearSectorIndex(&nearSectors, (int)(m_vPos.x), (int)(m_vPos.z));

	// 이동 후: 인접 섹터 순회 -> 유저가 있을 시 new viewlist 내에 등록
	for (auto& s : nearSectors)
	{
		// 인접 섹터 내의 타 유저들이 있는지 검사
		if (!(CSectorMgr::GetInstance()->Get_SectorList()[s.first][s.second].Get_ObjList().empty()))
		{
			// 타 유저의 서버 번호 추출
			for (auto obj_num : CSectorMgr::GetInstance()->Get_SectorList()[s.first][s.second].Get_ObjList())
			{
				/* 타유저일 경우 처리 */
				if (true == CObjMgr::GetInstance()->Is_Player(obj_num))
				{
					CPlayer* pPlayer = static_cast<CPlayer*>(CObjMgr::GetInstance()->Get_GameObject(L"PLAYER", obj_num));

					// 접속한 유저만 시야 목록에 등록한다.
					if (!pPlayer->Get_IsConnected()) continue;

					// 시야 내에 있다면 시야 목록에 등록한다.
					if (CObjMgr::GetInstance()->Is_Near(this, pPlayer))
						new_viewlist.insert(obj_num);
				}
			}
		}
	}

	// 이동 전 viewlist & 이동 후 viewlist 비교 -> 각 유저들의 시야 목록 내에 Monster 존재 여부를 결정.
	for (auto pl : old_viewlist)
	{
		// 이동 후에도 Monster 시야 목록 내에 "pl"(server number) 유저가 남아있는 경우
		if (0 < new_viewlist.count(pl))
		{
			CPlayer* pPlayer = static_cast<CPlayer*>(CObjMgr::GetInstance()->Get_GameObject(L"PLAYER", pl));
			if (pPlayer != nullptr)
			{
				/* 해당 유저의 시야 목록에 현재 Monster가 존재할 경우 */
				pPlayer->v_lock.lock();
				if (0 < pPlayer->view_list.count(m_sNum))
				{
					pPlayer->v_lock.unlock();
					/* 해당 유저에게 NPC가 움직인 후의 위치를 전송 */
					send_Monster_move_packet(pl, Monster_Normal::RUN);
				}
				/* 해당 유저의 시야 목록에 현재 Monster가 존재하지 않을 경우 */
				else
				{
					/* 해당 유저의 시야 목록에 현재 Monster 등록 */
					pPlayer->view_list.insert(m_sNum);
					pPlayer->v_lock.unlock();
					send_Monster_enter_packet(pl);
				}
			}
		}
		// 이동 후에 Monster 시야 목록 내에 "pl"(server number) 유저가 없는 경우
		else
		{
			CPlayer* pPlayer = static_cast<CPlayer*>(CObjMgr::GetInstance()->Get_GameObject(L"PLAYER", pl));
			if (pPlayer != nullptr)
			{
				/* 해당 유저의 시야 목록에 현재 Monster가 존재할 경우 */
				pPlayer->v_lock.lock();
				if (0 < pPlayer->view_list.count(m_sNum))
				{
					/* 해당 유저의 시야 목록에서 현재 Monster 삭제 */
					pPlayer->view_list.erase(m_sNum);
					pPlayer->v_lock.unlock();
					send_leave_packet(pl, m_sNum);
				}
				else
					pPlayer->v_lock.unlock();
			}
		}
	}

	// new_vielist 순회 -> 플레이어의 시야 목록에 있어야 할 새로운 Monster들을 추가
	for (auto pl : new_viewlist)
	{
		CPlayer* pPlayer = static_cast<CPlayer*>(CObjMgr::GetInstance()->Get_GameObject(L"PLAYER", pl));
		if (pPlayer != nullptr)
		{
			pPlayer->v_lock.lock();
			if (0 == pPlayer->view_list.count(pl))
			{
				/* 각 유저의 시야 목록 내에 현재 Monster가 없을 경우 -> 현재 Monster 등록 */
				if (0 == pPlayer->view_list.count(m_sNum))
				{
					pPlayer->view_list.insert(m_sNum);
					pPlayer->v_lock.unlock();
					send_Monster_enter_packet(pl);
				}
				/* 각 유저의 시야 목록 내에 현재 Monster가 있을 경우 -> 현재 Monster 위치 전송 */
				else
				{
					pPlayer->v_lock.unlock();
					send_Monster_move_packet(pl, Monster_Normal::RUN);
				}
			}
			else
				pPlayer->v_lock.unlock();
		}
	}
}

void CMonster::Attack_Crab(const float& fTimeDelta)
{
	/* 해당 Monster의 원래 위치값 */
	float ori_x, ori_y, ori_z;
	ori_x = m_vPos.x;
	ori_y = m_vPos.y;
	ori_z = m_vPos.z;
	 
	// 공격 전 위치에서의 viewlist (시야 내에 플레이어 저장)
	unordered_set<pair<int, int>> oldnearSector;
	oldnearSector.reserve(5);
	CSectorMgr::GetInstance()->Get_NearSectorIndex(&oldnearSector, (int)ori_x, (int)ori_z);

	unordered_set <int> old_viewlist;
	unordered_set <int> old_targetList;

	// 공격 전: 인접 섹터 순회 (몬스터 시야 파악)
	for (auto& s : oldnearSector)
	{
		// 인접 섹터 내의 타 유저들이 있는지 검사
		if (!(CSectorMgr::GetInstance()->Get_SectorList()[s.first][s.second].Get_ObjList().empty()))
		{
			// 유저의 서버 번호 추출
			for (auto obj_num : CSectorMgr::GetInstance()->Get_SectorList()[s.first][s.second].Get_ObjList())
			{
				/* 유저일 경우 처리 */
				if (true == CObjMgr::GetInstance()->Is_Player(obj_num))
				{
					CPlayer* pPlayer = static_cast<CPlayer*>(CObjMgr::GetInstance()->Get_GameObject(L"PLAYER", obj_num));

					// 접속한 유저만 시야 목록에 등록한다.
					if (!pPlayer->Get_IsConnected()) continue;

					// 시야 내에 있다면 시야 목록에 등록한다.
					if (CObjMgr::GetInstance()->Is_Near(this, pPlayer))
						old_viewlist.insert(obj_num);

					// 몬스터 공격 범위 내에 있는 유저 탐색한다.
					if (CObjMgr::GetInstance()->Is_Monster_AttackTarget(this, pPlayer, ATTACK_RANGE_CRAB))
						old_targetList.insert(obj_num);
				}
			}
		}
	}

	/* 타겟(공격 대상)이 존재할 경우 -> 몬스터 공격 시작 알리기 */
	if (!old_targetList.empty())
	{
		// Monster View List 내의 유저들에게 해당 Monster의 공격 시작을 알림.
		for (auto pl : old_viewlist)
		{
			/* 유저일 경우 처리 */
			if (true == CObjMgr::GetInstance()->Is_Player(pl))
			{
				if (!m_bIsAttack) return;
				send_Monster_NormalAttack(pl, Monster_Normal::ATTACK);
			}
		}		
		// 주변 유저에게 monster_attack_start를 알렸다면 잠시 공격 중지 -> 일정 시간 후 재공격
		Set_Stop_Attack();
	}
	/* 타겟(공격 대상)이 존재하지 않을 경우 -> 생성된 위치로 돌아감 */
	else
	{
		Change_ChaseMode();
	}
}

void CMonster::Attack_Monkey(const float& fTimeDelta)
{
	/* 해당 Monster의 원래 위치값 */
	float ori_x, ori_y, ori_z;
	ori_x = m_vPos.x;
	ori_y = m_vPos.y;
	ori_z = m_vPos.z;

	// 공격 전 위치에서의 viewlist (시야 내에 플레이어 저장)
	unordered_set<pair<int, int>> oldnearSector;
	oldnearSector.reserve(5);
	CSectorMgr::GetInstance()->Get_NearSectorIndex(&oldnearSector, (int)ori_x, (int)ori_z);

	unordered_set <int> old_viewlist;
	unordered_set <int> old_targetList;

	// 공격 전: 인접 섹터 순회 (몬스터 시야 파악)
	for (auto& s : oldnearSector)
	{
		// 인접 섹터 내의 타 유저들이 있는지 검사
		if (!(CSectorMgr::GetInstance()->Get_SectorList()[s.first][s.second].Get_ObjList().empty()))
		{
			// 유저의 서버 번호 추출
			for (auto obj_num : CSectorMgr::GetInstance()->Get_SectorList()[s.first][s.second].Get_ObjList())
			{
				/* 유저일 경우 처리 */
				if (true == CObjMgr::GetInstance()->Is_Player(obj_num))
				{
					CPlayer* pPlayer = static_cast<CPlayer*>(CObjMgr::GetInstance()->Get_GameObject(L"PLAYER", obj_num));

					// 접속한 유저만 시야 목록에 등록한다.
					if (!pPlayer->Get_IsConnected()) continue;

					// 시야 내에 있다면 시야 목록에 등록한다.
					if (CObjMgr::GetInstance()->Is_Near(this, pPlayer))
						old_viewlist.insert(obj_num);
				}
			}
		}
	}

	/* 타겟(공격 대상)이 존재할 경우 -> 몬스터 공격 시작 알리기 */
	if (m_iTargetNum != -1)
	{
		CPlayer* pTarget = static_cast<CPlayer*>(CObjMgr::GetInstance()->Get_GameObject(L"PLAYER", m_iTargetNum));
		if (pTarget == nullptr) return;

		m_vDir = pTarget->m_vPos - m_vPos;
		m_vDir.Normalize();

		/* 원거리 & 근거리 공격 타입 설정 */
		float fDist = Calculate_TargetDist(pTarget->m_vPos);
		if ((THROW_RANGE_MONKEY_START * THROW_RANGE_MONKEY_START) < fDist && fDist < (THROW_RANGE_MONKEY_END * THROW_RANGE_MONKEY_END))
		{
			m_bIsShortAttack = false;
		}
		else if ((ATTACK_RANGE_MONKEY * ATTACK_RANGE_MONKEY) < fDist)
		{
			Change_ChaseMode();
			return;
		}

		// Monster View List 내의 유저들에게 해당 Monster의 공격 시작을 알림.
		for (auto pl : old_viewlist)
		{
			/* 유저일 경우 처리 */
			if (true == CObjMgr::GetInstance()->Is_Player(pl))
			{
				if (!m_bIsAttack) return;

				if (m_bIsShortAttack)
					send_Monster_NormalAttack(pl, Monster_Normal::ATTACK);
				else
					send_Monster_NormalAttack(pl, Monkey::ATTACK_THROW);
			}
		}
		// 주변 유저에게 monster_attack_start를 알렸다면 잠시 공격 중지 -> 일정 시간 후 재공격
		Set_Stop_Attack();
	}
	/* 타겟(공격 대상)이 존재하지 않을 경우 -> 생성된 위치로 돌아감 */
	else
	{
		Change_ChaseMode();
	}
}

void CMonster::Attack_Cloder(const float& fTimeDelta)
{
	/* 해당 Monster의 원래 위치값 */
	float ori_x, ori_y, ori_z;
	ori_x = m_vPos.x;
	ori_y = m_vPos.y;
	ori_z = m_vPos.z;

	// 공격 전 위치에서의 viewlist (시야 내에 플레이어 저장)
	unordered_set<pair<int, int>> oldnearSector;
	oldnearSector.reserve(5);
	CSectorMgr::GetInstance()->Get_NearSectorIndex(&oldnearSector, (int)ori_x, (int)ori_z);

	unordered_set <int> old_viewlist;
	unordered_set <int> old_targetList;

	// 공격 전: 인접 섹터 순회 (몬스터 시야 파악)
	for (auto& s : oldnearSector)
	{
		// 인접 섹터 내의 타 유저들이 있는지 검사
		if (!(CSectorMgr::GetInstance()->Get_SectorList()[s.first][s.second].Get_ObjList().empty()))
		{
			// 유저의 서버 번호 추출
			for (auto obj_num : CSectorMgr::GetInstance()->Get_SectorList()[s.first][s.second].Get_ObjList())
			{
				/* 유저일 경우 처리 */
				if (true == CObjMgr::GetInstance()->Is_Player(obj_num))
				{
					CPlayer* pPlayer = static_cast<CPlayer*>(CObjMgr::GetInstance()->Get_GameObject(L"PLAYER", obj_num));

					// 접속한 유저만 시야 목록에 등록한다.
					if (!pPlayer->Get_IsConnected()) continue;

					// 시야 내에 있다면 시야 목록에 등록한다.
					if (CObjMgr::GetInstance()->Is_Near(this, pPlayer))
						old_viewlist.insert(obj_num);
				}
			}
		}
	}

	/* 타겟(공격 대상)이 존재할 경우 -> 몬스터 공격 시작 알리기 */
	if (m_iTargetNum != -1)
	{
		CPlayer* pTarget = static_cast<CPlayer*>(CObjMgr::GetInstance()->Get_GameObject(L"PLAYER", m_iTargetNum));
		if (pTarget == nullptr) return;

		m_vDir = pTarget->m_vPos - m_vPos;
		m_vDir.Normalize();

		/* 공격 스킬 설정 */
		float fDist = Calculate_TargetDist(pTarget->m_vPos);
		if ((ATTACK_RANGE_EPIC * ATTACK_RANGE_EPIC) < fDist)
		{
			Change_ChaseMode();
			return;
		}

		// Monster View List 내의 유저들에게 해당 Monster의 공격 시작을 알림.
		for (auto pl : old_viewlist)
		{
			/* 유저일 경우 처리 */
			if (true == CObjMgr::GetInstance()->Is_Player(pl))
			{
				if (!m_bIsAttack) return;
				
				send_Monster_NormalAttack(pl, rand() % 3 + 3);
			}
		}
		// 주변 유저에게 monster_attack_start를 알렸다면 잠시 공격 중지 -> 일정 시간 후 재공격
		Set_Stop_Attack();
	}
	/* 타겟(공격 대상)이 존재하지 않을 경우 -> 생성된 위치로 돌아감 */
	else
	{
		Change_ChaseMode();
	}
}

void CMonster::Attack_DrownedSailor(const float& fTimeDelta)
{
	/* 해당 Monster의 원래 위치값 */
	float ori_x, ori_y, ori_z;
	ori_x = m_vPos.x;
	ori_y = m_vPos.y;
	ori_z = m_vPos.z;

	// 공격 전 위치에서의 viewlist (시야 내에 플레이어 저장)
	unordered_set<pair<int, int>> oldnearSector;
	oldnearSector.reserve(5);
	CSectorMgr::GetInstance()->Get_NearSectorIndex(&oldnearSector, (int)ori_x, (int)ori_z);

	unordered_set <int> old_viewlist;
	unordered_set <int> old_targetList;

	// 공격 전: 인접 섹터 순회 (몬스터 시야 파악)
	for (auto& s : oldnearSector)
	{
		// 인접 섹터 내의 타 유저들이 있는지 검사
		if (!(CSectorMgr::GetInstance()->Get_SectorList()[s.first][s.second].Get_ObjList().empty()))
		{
			// 유저의 서버 번호 추출
			for (auto obj_num : CSectorMgr::GetInstance()->Get_SectorList()[s.first][s.second].Get_ObjList())
			{
				/* 유저일 경우 처리 */
				if (true == CObjMgr::GetInstance()->Is_Player(obj_num))
				{
					CPlayer* pPlayer = static_cast<CPlayer*>(CObjMgr::GetInstance()->Get_GameObject(L"PLAYER", obj_num));

					// 접속한 유저만 시야 목록에 등록한다.
					if (!pPlayer->Get_IsConnected()) continue;

					// 시야 내에 있다면 시야 목록에 등록한다.
					if (CObjMgr::GetInstance()->Is_Near(this, pPlayer))
						old_viewlist.insert(obj_num);
				}
			}
		}
	}

	/* 타겟(공격 대상)이 존재할 경우 -> 몬스터 공격 시작 알리기 */
	if (m_iTargetNum != -1)
	{
		CPlayer* pTarget = static_cast<CPlayer*>(CObjMgr::GetInstance()->Get_GameObject(L"PLAYER", m_iTargetNum));
		if (pTarget == nullptr) return;

		m_vDir = pTarget->m_vPos - m_vPos;
		m_vDir.Normalize();

		/* 공격 스킬 설정 */
		float fDist = Calculate_TargetDist(pTarget->m_vPos);
		if ((ATTACK_RANGE_CRAB * ATTACK_RANGE_CRAB) < fDist)
		{
			Change_ChaseMode();
			return;
		}

		// Monster View List 내의 유저들에게 해당 Monster의 공격 시작을 알림.
		for (auto pl : old_viewlist)
		{
			/* 유저일 경우 처리 */
			if (true == CObjMgr::GetInstance()->Is_Player(pl))
			{
				if (!m_bIsAttack) return;
				int ani = rand() % 5 + 3;
				Set_AnimationKey(ani);
				send_Monster_NormalAttack(pl, ani);
			}
		}
		// 주변 유저에게 monster_attack_start를 알렸다면 잠시 공격 중지 -> 일정 시간 후 재공격
		Set_Stop_Attack();
	}
	/* 타겟(공격 대상)이 존재하지 않을 경우 -> 생성된 위치로 돌아감 */
	else
	{
		Change_ChaseMode();
	}
}

void CMonster::Hurt_Monster(const int& p_id,const int& damage)
{
	/* 해당 Monster의 원래 위치값 */
	float ori_x, ori_y, ori_z;
	ori_x = m_vPos.x;
	ori_y = m_vPos.y;
	ori_z = m_vPos.z;

	// 피격당하기 전 위치에서의 viewlist (시야 내에 플레이어 저장)
	unordered_set<pair<int, int>> oldnearSector;
	oldnearSector.reserve(5);
	CSectorMgr::GetInstance()->Get_NearSectorIndex(&oldnearSector, (int)ori_x, (int)ori_z);

	unordered_set <int> old_viewlist;

	// 피격당하기 전: 인접 섹터 순회 (몬스터 시야 파악)
	for (auto& s : oldnearSector)
	{
		// 인접 섹터 내의 타 유저들이 있는지 검사
		if (!(CSectorMgr::GetInstance()->Get_SectorList()[s.first][s.second].Get_ObjList().empty()))
		{
			// 유저의 서버 번호 추출
			for (auto obj_num : CSectorMgr::GetInstance()->Get_SectorList()[s.first][s.second].Get_ObjList())
			{
				/* 유저일 경우 처리 */
				if (true == CObjMgr::GetInstance()->Is_Player(obj_num))
				{
					CPlayer* pPlayer = static_cast<CPlayer*>(CObjMgr::GetInstance()->Get_GameObject(L"PLAYER", obj_num));

					// 접속한 유저만 시야 목록에 등록한다.
					if (!pPlayer->Get_IsConnected()) continue;

					// 시야 내에 있다면 시야 목록에 등록한다.
					if (CObjMgr::GetInstance()->Is_Near(this, pPlayer))
						old_viewlist.insert(obj_num);
				}
			}
		}
	}

	/* 피격 당함 */
	if (m_iHp >= 0)
	{
		m_iHp -= damage;
		if (m_iHp < 0)
			m_iHp = 0;
	}

	// Monster View List 내의 유저들에게 해당 Monster의 변경된 stat을 알림.
	for (auto pl : old_viewlist)
	{
		/* 유저일 경우 처리 */
		if (true == CObjMgr::GetInstance()->Is_Player(pl))
		{
			if (m_bIsDead) return;
			send_Monster_Stat(pl);
		}
	}

	/* Monster Attack Target 설정 */
	m_iTargetNum = p_id;

	/* 전투 상태로 변경 */
	Change_ChaseMode();
}

void CMonster::Change_AttackMode()
{
	/* Monster가 활성화되어 있지 않을 경우 활성화 */
	if (m_status != ST_ATTACK)
	{
		STATUS prev_state = m_status;
		if (true == atomic_compare_exchange_strong(&m_status, &prev_state, ST_ATTACK))
			Set_Start_Attack();
	}
}

void CMonster::Change_ChaseMode()
{
	/* Monster가 활성화되어 있지 않을 경우 활성화 */
	if (m_status != ST_CHASE)
	{
		STATUS prev_state = m_status;
		atomic_compare_exchange_strong(&m_status, &prev_state, ST_CHASE);
	}
}

float CMonster::Calculate_TargetDist(const _vec3& vPos)
{
	float dist = (vPos.x - m_vPos.x) * (vPos.x - m_vPos.x);
	dist += (vPos.y - m_vPos.y) * (vPos.y - m_vPos.y);
	dist += (vPos.z - m_vPos.z) * (vPos.z - m_vPos.z);

	return dist;
}

void CMonster::Play_Animation(float fTimeDelta)
{
	if (m_uiCurAniIndex >= m_uiNumAniIndex) return;

	/*__________________________________________________________________________________________________________
	[ 애니메이션이 계속 반복되도록 fmod 수행 ]
	____________________________________________________________________________________________________________*/
	if (m_uiNewAniIndex != m_uiCurAniIndex)
	{
		m_fAnimationTime = m_fBlendAnimationTime;
		m_fBlendingTime -= 0.001f * fTimeDelta;

		if (m_fBlendingTime <= 0.0f)
			m_fBlendingTime = 0.0f;
	}
	else
	{
		m_fAnimationTime += fTimeDelta;
	}

	m_fAnimationTime = (float)(fmod(m_fAnimationTime, (m_arrDuration[m_uiCurAniIndex])));

	/*__________________________________________________________________________________________________________
	[ 3DMax 상에서의 Frame 계산 ]
	____________________________________________________________________________________________________________*/
	m_ui3DMax_NumFrame = (_uint)(_3DMAX_FPS * (m_arrDuration[m_uiCurAniIndex] / Monster_Normal::TPS));
	m_ui3DMax_CurFrame = (_uint)(_3DMAX_FPS * (m_fAnimationTime / Monster_Normal::TPS));

	if (m_fBlendingTime <= 0.0f)
	{
		m_uiCurAniIndex = m_uiNewAniIndex;
		m_fAnimationTime = 0.0f;
		m_fBlendingTime = 1.f;
	}

}

void CMonster::Set_AnimationKey(const _uint& uiAniKey)
{
	if (m_uiNewAniIndex != uiAniKey)
	{
		m_uiNewAniIndex = uiAniKey;
		m_fBlendingTime = 1.0f;
		m_fBlendAnimationTime = m_fAnimationTime;
	}
}

bool CMonster::Is_AnimationSetEnd(const float& fTimeDelta)
{
	if ((m_fAnimationTime >= m_arrDuration[m_uiCurAniIndex] -
		Monster_Normal::TPS * ANIMA_INTERPOLATION * fTimeDelta) &&
		(m_uiCurAniIndex == m_uiNewAniIndex))
	{
		return true;
	}

	return false;
}

void CMonster::Set_Stop_Attack()
{
	if (m_bIsAttack)
	{
		bool prev_state = m_bIsAttack;

		if (true == atomic_compare_exchange_strong(reinterpret_cast<volatile atomic_bool*>(&m_bIsAttack), &prev_state, false))
			add_timer(m_sNum, OP_MODE_ATTACK_MONSTER, system_clock::now() + 5s);
	}
}

void CMonster::Set_Start_Attack()
{
	if (!m_bIsAttack && !m_bIsComeBack)
	{
		bool prev_state = m_bIsAttack;
		atomic_compare_exchange_strong(reinterpret_cast<volatile atomic_bool*>(&m_bIsAttack), &prev_state, true);		
	}
}

void CMonster::send_Monster_enter_packet(int to_client)
{
	sc_packet_monster_enter p;

	p.size = sizeof(p);
	p.type = SC_PACKET_MONSTER_ENTER;
	p.id = m_sNum;

	c_lock.lock();
	strncpy_s(p.name, m_ID, strlen(m_ID));
	strncpy_s(p.naviType, m_naviType, strlen(m_naviType));
	c_lock.unlock();

	p.mon_num = m_monNum;

	p.posX = m_vPos.x;
	p.posY = m_vPos.y;
	p.posZ = m_vPos.z;

	p.angleX = m_vAngle.x;
	p.angleY = m_vAngle.y;
	p.angleZ = m_vAngle.z;

	p.Hp = m_iHp;
	p.maxHp = m_iMaxHp;
	p.spd = m_fSpd;

	send_packet(to_client, &p);
}

void CMonster::send_Monster_move_packet(int to_client, int ani)
{
	sc_packet_move p;

	p.size = sizeof(p);
	p.type = SC_PACKET_MONSTER_MOVE;
	p.id = m_sNum;

	p.animIdx = ani;
	p.spd  = m_fSpd;

	p.posX = m_vPos.x;
	p.posY = m_vPos.y;
	p.posZ = m_vPos.z;

	p.dirX = m_vDir.x;
	p.dirY = m_vDir.y;
	p.dirZ = m_vDir.z;

	send_packet(to_client, &p);
}

void CMonster::send_Monster_NormalAttack(int to_client,int ani)
{
	sc_packet_monster_attack p;

	p.size = sizeof(p);
	p.type = SC_PACKET_MONSTER_ATTACK;
	p.id = m_sNum;

	p.animIdx = ani;

	p.dirX = m_vDir.x;
	p.dirY = m_vDir.y;
	p.dirZ = m_vDir.z;

	send_packet(to_client, &p);
}

void CMonster::send_Monster_Stat(int to_client)
{
	sc_packet_stat_change p;

	p.size = sizeof(p);
	p.type = SC_PACKET_MONSTER_STAT;
	p.id = m_sNum;

	p.hp = m_iHp;
	p.mp = 0;
	p.exp = m_iExp;

	send_packet(to_client, &p);
}

DWORD CMonster::Release()
{
	return 0;
}
