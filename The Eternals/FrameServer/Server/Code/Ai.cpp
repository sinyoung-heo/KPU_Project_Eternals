#include "stdafx.h"
#include "Ai.h"
#include "Player.h"
#include "Monster.h"

CAi::CAi()
    :m_iLevel(0), m_iHp(0), m_iMaxHp(0), m_fSpd(0.f), m_bIsAttack(false), m_bIsReaction(false),
    m_iMp(0), m_iMaxMp(0), m_iMaxAtt(0), m_iMinAtt(0), m_bIsMove(false),
    m_bIsAttackStance(false), m_bIsPartyState(false), m_iPartyNumber(-1)
{
	memset(m_arrAttackPattern, -1, sizeof(int) * VERGOS_PATTERN);
}

CAi::~CAi()
{
}

void CAi::Set_AnimDuration(double arr[])
{
	for (_uint i = 0; i < MAX_ANI_AI; ++i)
	{
		if (m_uiNumAniIndex > i)
			m_arrDuration[i] = arr[i];
	}
}

void CAi::Set_AnimationKey(const _uint& uiAniKey)
{
	if (m_uiNewAniIndex != uiAniKey)
	{
		m_uiNewAniIndex = uiAniKey;
		m_fBlendingTime = 1.0f;
		m_fBlendAnimationTime = m_fAnimationTime;
	}
}

void CAi::Set_Stop_Attack(chrono::seconds t)
{
	/* 공격이 끝나면 추적 모드로 변경 */
	if (m_bIsAttack)
	{
		bool prev_state = m_bIsAttack;

		if (true == atomic_compare_exchange_strong(reinterpret_cast<volatile atomic_bool*>(&m_bIsAttack), &prev_state, false))
		{
			add_timer(m_sNum, OP_MODE_CHASE_AI, system_clock::now() + t);
		}
	}
}

void CAi::Set_Start_Attack()
{
	if (!m_bIsAttack)
	{
		bool prev_state = m_bIsAttack;
		atomic_compare_exchange_strong(reinterpret_cast<volatile atomic_bool*>(&m_bIsAttack), &prev_state, true);
	}
}

void CAi::Set_Stop_Reaction(chrono::seconds t)
{
	if (m_bIsReaction)
	{
		bool prev_state = m_bIsReaction;
		if (true == atomic_compare_exchange_strong(reinterpret_cast<volatile atomic_bool*>(&m_bIsReaction), &prev_state, false))
		{
			//add_timer(m_sNum, OP_MODE_CHASE_MONSTER, system_clock::now() + t);
		}
	}
}

void CAi::Set_Start_Reaction()
{
	if (!m_bIsReaction)
	{
		bool prev_state = m_bIsReaction;
		atomic_compare_exchange_strong(reinterpret_cast<volatile atomic_bool*>(&m_bIsReaction), &prev_state, true);
	}
}

void CAi::Ready_AI(const char& chJob, const char& chWeaponType, const char& chStageID, const _vec3& vPos)
{
    m_sNum += AI_NUM_START;
    int s_num = m_sNum;

    m_bIsConnect        = true;
    m_bIsDead           = false;

	sprintf_s(m_ID, "AI_%03d", m_sNum);
    m_type              = chJob;
    m_chWeaponType      = chWeaponType;
    m_chStageId         = chStageID;
    m_bIsAttackStance   = true;
    m_bIsPartyState     = true;
    m_iPartyNumber      = RAID_PARTY;

    m_iLevel            = INIT_LEV;
    m_iHp               = AI_HP;
    m_iMaxHp            = AI_HP;
    m_iMp               = AI_MP;
    m_iMaxMp            = AI_MP;
    m_iMinAtt           = AI_MINATT;
    m_iMaxAtt           = AI_MAXATT;
    m_fSpd              = INIT_SPEED;

    m_vPos              = vPos;
    m_vDir              = _vec3(0.f, 0.f, 1.f);
    m_vAngle            = _vec3(0.f, 0.f, 0.f);
	m_status			= STATUS::ST_NONACTIVE;

	// Move Speed
	m_tMoveSpeedInterpolationDesc.linear_ratio = 0.0f;
	m_tMoveSpeedInterpolationDesc.v1 = Archer::MIN_SPEED;
	m_tMoveSpeedInterpolationDesc.v2 = Archer::MAX_SPEED * Archer::OTHERS_OFFSET;

    CSectorMgr::GetInstance()->Enter_ClientInSector(s_num, (int)(m_vPos.z / SECTOR_SIZE), (int)(m_vPos.x / SECTOR_SIZE));
    CObjMgr::GetInstance()->Add_GameObject(L"AI", this, s_num);
    CObjMgr::GetInstance()->Add_PartyMember(RAID_PARTY, &m_iPartyNumber, s_num);
}

void CAi::Release_AI()
{
    process_disconnect_ai();
}

int CAi::Update_AI(const float& fTimeDelta)
{
	if (fTimeDelta > 1.f)
		return NO_EVENT;

	Change_Animation(fTimeDelta);

	/* Calculate Animation frame */
	Set_AnimationKey(m_uiAnimIdx);
	Play_Animation(fTimeDelta * Monster_Normal::TPS);

	return NO_EVENT;
}

void CAi::active_AI()
{
	if (m_status == ST_NONACTIVE)
	{
		STATUS prev_state = m_status;
		atomic_compare_exchange_strong(&m_status, &prev_state, ST_ACTIVE);
	}
}

void CAi::Change_AttackMode()
{
	if (m_status != ST_ATTACK)
	{
		STATUS prev_state = m_status;
		if (true == atomic_compare_exchange_strong(&m_status, &prev_state, ST_ATTACK))
			Set_Start_Attack();
	}
}

void CAi::Change_ChaseMode()
{
	if (m_status != ST_CHASE)
	{
		STATUS prev_state = m_status;
		atomic_compare_exchange_strong(&m_status, &prev_state, ST_CHASE);
	}
}

void CAi::Change_ReactionMode()
{
	if (m_status != ST_REACTION)
	{
		STATUS prev_state = m_status;
		if (true == atomic_compare_exchange_strong(&m_status, &prev_state, ST_REACTION))
			Set_Start_Reaction();
	}
}

void CAi::Change_Animation(const float& fTimeDelta)
{
	switch (m_type)
	{
	case PC_GLADIATOR:
	{

	}
	break;

	case PC_ARCHER:
	{
		Change_Archer_Animation(fTimeDelta);
	}
	break;

	case PC_PRIEST:
	{

	}
	break;
	}
}

void CAi::Change_Archer_Animation(const float& fTimeDelta)
{
	switch (m_status)
	{

	case STATUS::ST_ACTIVE:
	{
		if (m_bIsMove)
		{
			if (false == CCollisionMgr::GetInstance()->Is_Arrive(m_vPos, m_vTempPos))
			{
				m_tMoveSpeedInterpolationDesc.interpolation_speed = 1.0f * Archer::OTHERS_OFFSET;
				m_tMoveSpeedInterpolationDesc.linear_ratio += m_tMoveSpeedInterpolationDesc.interpolation_speed * fTimeDelta;
				m_fSpd = LinearInterpolation(m_tMoveSpeedInterpolationDesc.v1, m_tMoveSpeedInterpolationDesc.v2, m_tMoveSpeedInterpolationDesc.linear_ratio);

				m_vDir = m_vTempPos - m_vPos;
				m_vDir.Normalize();

				m_vPos += m_vDir * m_fSpd * fTimeDelta;
			}	
			else
			{
				m_bIsMove = false;
				process_moveStop_archer(fTimeDelta);
				Change_ChaseMode();
			}
		}
		else
			process_move_archer(fTimeDelta);

	}
	break;

	case STATUS::ST_NONACTIVE:
	{
		m_iAniIdx = Archer::ATTACK_WAIT;
	}
	break;

	case STATUS::ST_CHASE:
	{
		Choose_ArcherPattern(fTimeDelta);
	}
	break;

	case STATUS::ST_ATTACK:
	{
		Attack_Archer_AI(fTimeDelta);
	}
	break;

	case STATUS::ST_REACTION:
	{
		Play_Archer_NextAttack();
	}
	break;

	case STATUS::ST_DEAD:
	{
		
	}
	break;
	}
}

void CAi::process_move_archer(const float& fTimeDelta)
{
	if (m_bIsMove) return;

	// 움직일 방향 설정
	CMonster* pMonster = static_cast<CMonster*>(CObjMgr::GetInstance()->Get_GameObject(L"MONSTER", g_iVergosServerNum));
	if (pMonster->Get_Dead()) return;

	m_vDir = pMonster->m_vPos - m_vPos;
	m_vDir.Normalize();

	m_vTempPos = m_vPos + m_vDir * 10.f;
	m_vTempPos.z = 350.f;

	m_iAniIdx = Archer::ATTACK_RUN;
	m_uiAnimIdx = Archer::ATTACK_RUN;

	m_bIsMove = true;

	for (const int& raid : *CObjMgr::GetInstance()->Get_RAIDLIST())
	{
		CPlayer* pPlayer = static_cast<CPlayer*>(CObjMgr::GetInstance()->Get_GameObject(L"PLAYER", raid));
		if (pPlayer == nullptr || pPlayer->Get_IsConnected() == false) return;

		send_AI_move_packet(raid);
	}
}

void CAi::process_moveStop_archer(const float& fTimeDelta)
{
	// 움직일 방향 설정
	m_iAniIdx = Archer::ATTACK_WAIT;
	m_uiAnimIdx = Archer::ATTACK_WAIT;
	
	for (const int& raid : *CObjMgr::GetInstance()->Get_RAIDLIST())
	{
		CPlayer* pPlayer = static_cast<CPlayer*>(CObjMgr::GetInstance()->Get_GameObject(L"PLAYER", raid));
		if (pPlayer == nullptr || pPlayer->Get_IsConnected() == false) return;

		send_AI_moveStop_packet(raid);
	}
}

void CAi::Choose_ArcherPattern(const float& fTimeDelta)
{
	if (rand() % 5 == 0)
		ArcherPattern_FirstPhase();
	else  
		ArcherPattern_SecondPhase();

	m_iCurPatternNumber = 0;
	Change_AttackMode();
}

void CAi::ArcherPattern_FirstPhase()
{
	m_arrAttackPattern[0] = Archer::ATTACK_ARROW;
	m_arrAttackPattern[1] = Archer::RAPID_SHOT1;
	m_arrAttackPattern[2] = Archer::ARROW_FALL_START;
	m_arrAttackPattern[3] = Archer::BACK_DASH;
	m_arrAttackPattern[4] = Archer::ARROW_SHOWER_START;
	m_arrAttackPattern[5] = Archer::CHARGE_ARROW_START;
	m_arrAttackPattern[6] = Archer::RAPID_SHOT1;
}

void CAi::ArcherPattern_SecondPhase()
{
	m_arrAttackPattern[0] = Archer::ATTACK_ARROW;
	m_arrAttackPattern[1] = Archer::RAPID_SHOT1;
	m_arrAttackPattern[2] = Archer::ARROW_FALL_START;
	m_arrAttackPattern[3] = Archer::BACK_DASH;
	m_arrAttackPattern[4] = Archer::ARROW_SHOWER_START;
	m_arrAttackPattern[5] = Archer::CHARGE_ARROW_START;
	m_arrAttackPattern[6] = Archer::RAPID_SHOT1;
}

void CAi::Attack_Archer_AI(const float& fTimedelta)
{
	if (!m_bIsAttack) return;

	if (Archer::BACK_DASH <= m_uiAnimIdx && m_uiAnimIdx <= Archer::CHARGE_ARROW_SHOT)
	{
		if (!Is_AnimationSetEnd(fTimedelta))
			return;
		else
		{
			++m_iCurPatternNumber;
			m_uiAnimIdx = Archer::ATTACK_WAIT;

			if (m_iCurPatternNumber >= VERGOS_PATTERN)
				Set_Stop_Attack(1s);
			else
				Change_ReactionMode();
			return;
		}
	}

	m_uiAnimIdx = m_arrAttackPattern[m_iCurPatternNumber];
	m_iAniIdx = m_arrAttackPattern[m_iCurPatternNumber];

	for (const int& raid : *CObjMgr::GetInstance()->Get_RAIDLIST())
	{
		CPlayer* pPlayer = static_cast<CPlayer*>(CObjMgr::GetInstance()->Get_GameObject(L"PLAYER", raid));
		if (pPlayer == nullptr || pPlayer->Get_IsConnected() == false) return;

		send_AI_attack_packet(raid);
	}
}

void CAi::Play_Archer_NextAttack(chrono::seconds t)
{
	if (m_bIsReaction)
	{
		bool prev_state = m_bIsReaction;
		if (true == atomic_compare_exchange_strong(reinterpret_cast<volatile atomic_bool*>(&m_bIsReaction), &prev_state, false))
		{
			add_timer(m_sNum, OP_MODE_AI_NEXT_ATTACK, system_clock::now() + t);
		}
	}
}

void CAi::process_disconnect_ai()
{
	/* sector에서 해당 플레이어 지우기 */
	CSectorMgr::GetInstance()->Leave_ClientInSector(m_sNum, (int)(m_vPos.z / SECTOR_SIZE), (int)(m_vPos.x / SECTOR_SIZE));

	/* 파티에 가입되어 있다면 파티 탈퇴 */
	if (m_bIsPartyState == true)
		process_leave_party_ai();

	/* 해당 플레이어가 등록되어 있는 섹터 내의 유저들에게 접속 종료를 알림 */
	unordered_set<pair<int, int>> nearSector;
	nearSector.reserve(NEAR_SECTOR);
	CSectorMgr::GetInstance()->Get_NearSectorIndex(&nearSector, (int)m_vPos.x, (int)m_vPos.z);

	// 인접 섹터 순회
	for (auto& s : nearSector)
	{
		// 인접 섹터 내의 타 유저들이 있는지 검사
		if (!(CSectorMgr::GetInstance()->Get_SectorList()[s.first][s.second].Get_ObjList().empty()))
		{
			// 타 유저의 서버 번호 추출
			for (auto obj_num : CSectorMgr::GetInstance()->Get_SectorList()[s.first][s.second].Get_ObjList())
			{
				/* 오직 유저들에게만 패킷을 전송 (NPC 제외) */
				if (false == CObjMgr::GetInstance()->Is_Player(obj_num)) continue;
				// '나'에게 패킷 전송 X
				if (obj_num == m_sNum) continue;

				CPlayer* pOther = static_cast<CPlayer*>(CObjMgr::GetInstance()->Get_GameObject(L"PLAYER", obj_num));

				// 접속한 유저들에게만 접속 종료를 알림
				if (pOther->Get_IsConnected())
				{
					/* 타 유저의 시야 목록 내에 '나'가 있다면 지운다 */
					pOther->v_lock.lock();
					if (0 != pOther->view_list.count(m_sNum))
					{
						pOther->view_list.erase(m_sNum);
						pOther->v_lock.unlock();

						/* 타 유저에게 접속 종료 패킷 전송 */
#ifdef TEST
						cout << obj_num << "님에게" << id << "님 퇴장을 전송" << endl;
#endif					
						send_leave_packet(obj_num, m_sNum);
					}
					else pOther->v_lock.unlock();
				}
			}
		}
	}


	m_bIsConnect	= false;
	m_bIsDead		= false;
	m_vPos			= _vec3(0.f, 0.f, 0.f);
	m_vDir			= _vec3(0.f, 0.f, 0.f);
	m_ID[0]			= 0;
	m_type			= 0;
	m_chStageId		= STAGE_WINTER;
	m_bIsPartyState = false;
	m_iPartyNumber	= INIT_PARTY_NUMBER;
	m_status		= STATUS::ST_END;

	CObjPoolMgr::GetInstance()->return_Object(L"AI", this);
	CObjMgr::GetInstance()->Delete_GameObject(L"AI", this);
}

void CAi::process_leave_party_ai()
{
	if (!m_bIsPartyState) return;

	// 파티 구성원들에게 파티 탈퇴를 알림
	for (auto& p : *CObjMgr::GetInstance()->Get_PARTYLIST(m_iPartyNumber))
	{
		if (p != m_sNum && CObjMgr::GetInstance()->Is_Player(p))
		{
			// 탈퇴 멤버 정보 -> 기존 구성원
			send_leave_party_ai(p);
		}
	}

	// 해당 유저의 파티 정보 초기화
	CObjMgr::GetInstance()->Leave_Party(&m_iPartyNumber, m_sNum);
	m_bIsPartyState = false;
}

void CAi::Play_Animation(const float& fTimeDelta)
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

bool CAi::Is_AnimationSetEnd(const float& fTimeDelta)
{
	if ((m_fAnimationTime >= m_arrDuration[m_uiCurAniIndex] -
		static_cast<double>(Monster_Normal::TPS * ANIMA_INTERPOLATION * fTimeDelta)) &&
		(m_uiCurAniIndex == m_uiNewAniIndex))
	{
		return true;
	}

	return false;
}

void CAi::send_AI_enter_packet(const int& to_client)
{
	sc_packet_enter p;

	p.size				= sizeof(p);
	p.type				= SC_PACKET_ENTER;
	p.id				= m_sNum;

	c_lock.lock();
	strncpy_s(p.name, m_ID, strlen(m_ID));
	c_lock.unlock();

	p.o_type			= m_type;
	p.weaponType		= m_chWeaponType;
	p.stageID			= m_chStageId;
	p.is_stance_attack	= m_bIsAttackStance;
	p.is_party_state	= m_bIsPartyState;

	p.posX				= m_vPos.x;
	p.posY				= m_vPos.y;
	p.posZ				= m_vPos.z;

	p.dirX				= m_vDir.x;
	p.dirY				= m_vDir.y;
	p.dirZ				= m_vDir.z;
	
	send_packet(to_client, &p);
}

void CAi::send_AI_move_packet(const int& to_client)
{
	sc_packet_move p;

	p.size		= sizeof(p);
	p.type		= SC_PACKET_MOVE;
	p.id		= m_sNum;

	p.spd		= m_fSpd;
	p.animIdx	= m_iAniIdx;
	p.move_time = move_time;

	p.posX		= m_vTempPos.x;
	p.posY		= m_vTempPos.y;
	p.posZ		= m_vTempPos.z;

	p.dirX		= m_vDir.x;
	p.dirY		= m_vDir.y;
	p.dirZ		= m_vDir.z;

	send_packet(to_client, &p);
}

void CAi::send_AI_moveStop_packet(const int& to_client)
{
	sc_packet_move p;

	p.size = sizeof(p);
	p.type = SC_PACKET_MOVE_STOP;
	p.id = m_sNum;

	p.spd = m_fSpd;
	p.animIdx = m_iAniIdx;
	p.move_time = move_time;

	p.posX = m_vPos.x;
	p.posY = m_vPos.y;
	p.posZ = m_vPos.z;

	p.dirX = m_vDir.x;
	p.dirY = m_vDir.y;
	p.dirZ = m_vDir.z;

	send_packet(to_client, &p);
}

void CAi::send_AI_attack_packet(const int& to_client)
{
	sc_packet_attack p;

	p.size = sizeof(p);
	p.type = SC_PACKET_ATTACK;
	p.id = m_sNum;

	p.o_type = m_type;

	p.posX = m_vPos.x;
	p.posY = m_vPos.y;
	p.posZ = m_vPos.z;

	p.dirX = m_vDir.x;
	p.dirY = m_vDir.y;
	p.dirZ = m_vDir.z;

	p.animIdx = m_iAniIdx;
	p.end_angleY = -1.f;

	send_packet(to_client, &p);
}

void CAi::send_leave_party_ai(const int& to_client)
{
	sc_packet_suggest_party p;

	p.size = sizeof(p);
	p.type = SC_PACKET_LEAVE_PARTY;
	p.id = m_sNum;

	send_packet(to_client, &p);
}

DWORD CAi::Release()
{
    return 0;
}
