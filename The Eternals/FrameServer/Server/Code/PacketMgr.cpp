#include "stdafx.h"
#include "PacketMgr.h"
#include "Player.h"
#include "Npc.h"

/* ========================패킷 처리========================*/
void process_packet(int id)
{
	/* Server Number 해당 유저 */
	CPlayer* pPlayer = static_cast<CPlayer*>(CObjMgr::GetInstance()->Get_GameObject(L"PLAYER", id));

	if (pPlayer == nullptr) return;

	// 패킷 타입
	char p_type = pPlayer->m_packet_start[1];

	switch (p_type)
	{
	case CS_LOGIN:
	{
		cs_packet_login* p = reinterpret_cast<cs_packet_login*>(pPlayer->m_packet_start);

		pPlayer->Get_ClientLock().lock();
		strcpy_s(pPlayer->m_ID, p->name);
		pPlayer->Get_ClientLock().unlock();

		/* CHECK ID in Database Server */
		if (false == CDBMgr::GetInstance()->Check_ID(id, p->password))
		{
			/* 처음 게임에 접속한 유저라면 회원으로 등록 in Database Server */
			CDBMgr::GetInstance()->Insert_NewPlayer_DB(id, p->password);
		}

		/* 로그인 수락 패킷 전송 */
		send_login_ok(id);

		/* Sector 다시 등록 (접속 시 미리 한 번 하고있음. 완전함을 위해 한 번 더 등록(sector의 Key는 Unique함) */
		CSectorMgr::GetInstance()->Enter_ClientInSector(id, (int)(pPlayer->m_vPos.y / SECTOR_SIZE), (int)(pPlayer->m_vPos.x / SECTOR_SIZE));

		/* 해당 플레이어가 등록되어 있는 섹터 내의 유저들에게 접속을 알림 */
		unordered_set<pair<int, int>> nearSector;
		nearSector.reserve(5);
		CSectorMgr::GetInstance()->Get_NearSectorIndex(&nearSector, (int)pPlayer->m_vPos.x, (int)pPlayer->m_vPos.y);

		// 인접 섹터 순회
		for (auto& s : nearSector)
		{
			// 인접 섹터 내의 타 유저들이 있는지 검사
			if (!(CSectorMgr::GetInstance()->Get_SectorList()[s.first][s.second].Get_ObjList().empty()))
			{
				// 타 유저의 서버 번호 추출
				for (auto obj_num : CSectorMgr::GetInstance()->Get_SectorList()[s.first][s.second].Get_ObjList())
				{
					// '나'에게 등장 패킷 전송 X
					if (obj_num == id) continue;

					/* 타유저일 경우 처리 */
					if (true == CObjMgr::GetInstance()->Is_Player(obj_num))
					{
						CPlayer* pOther = static_cast<CPlayer*>(CObjMgr::GetInstance()->Get_GameObject(L"PLAYER", obj_num));

						// 접속한 유저들에게만 접속 종료를 알림
						if (pOther->Get_IsConnected())
						{
							// 시야 내에 없다면 시야 목록에 등록X.
							if (false == CObjMgr::GetInstance()->Is_Near(pPlayer, pOther)) continue;
							
							/* 타 유저의 시야에 내가 없을 경우 '나'를 등록시킴 */
							pOther->v_lock.lock();
							if (0 == pOther->view_list.count(id))
							{
								pOther->view_list.insert(id);
								pOther->v_lock.unlock();
								send_enter_packet(obj_num, id);
							}
							else pOther->v_lock.unlock();

							/* '나'의 시야에 타 유저가 없을 경우 -> 타 유저 등록 */
							pPlayer->v_lock.lock();
							if (0 == pPlayer->view_list.count(obj_num))
							{
								pPlayer->view_list.insert(obj_num);
								pPlayer->v_lock.unlock();
								send_enter_packet(id, obj_num);
							}
							else pPlayer->v_lock.unlock();
						}
					}
					/* NPC일 경우 처리 */
					else if (true == CObjMgr::GetInstance()->Is_NPC(obj_num))
					{
						CNpc* pNPC = static_cast<CNpc*>(CObjMgr::GetInstance()->Get_GameObject(L"NPC", obj_num));

						// 시야 내에 없다면 시야 목록에 등록X.
						if (false == CObjMgr::GetInstance()->Is_Near(pPlayer, pNPC)) continue;
						pPlayer->v_lock.lock();
						pPlayer->view_list.insert(obj_num);
						pPlayer->v_lock.unlock();
						send_NPC_enter_packet(id, obj_num);
					}
					/* MONSTER일 경우 처리*/
					else
					{
						int i = 0;
					}
				}
			}
		}
	}
	break;

	case CS_MOVE:
	{
		cs_packet_move* p = reinterpret_cast<cs_packet_move*>(pPlayer->m_packet_start);
		//pPlayer->move_time = p->move_time;
		process_move(id, p);
	}
	break;

	case CS_ATTACK:
		break;
	case CS_CHAT:
		break;
	case CS_LOGOUT:
		break;
	case CS_TELEPORT:
		break;
	}
}
/* ========================패킷 재조립========================*/
void process_recv(int id, DWORD iosize)
{
	/* Server Number 해당 유저 */
	CPlayer* pPlayer = static_cast<CPlayer*>(CObjMgr::GetInstance()->Get_GameObject(L"PLAYER", id));

	if (pPlayer == nullptr) return;
	
	// m_packet_start		: 처리할 데이터 버퍼 및 위치 (= iocp_buffer)
	// m_packet_start[0]	: 처리할 패킷의 크기
	unsigned char p_size = pPlayer->m_packet_start[0];
	
	/* 다음 패킷을 받을 링버퍼 위치 설정 */
	// m_recv_start			: 수신한 패킷의 시작 위치
	unsigned char* next_recv_ptr = pPlayer->m_recv_start + iosize;

	// 처리할 패킷 크기보다 남아있는 공간이 적당할 경우 패킷 처리 시작
	while (p_size <= next_recv_ptr - pPlayer->m_packet_start)
	{
		// 패킷 처리 (= 패킷 만들기)
		process_packet(id);
		// 패킷 처리 후 유저의 데이터 처리 버퍼 시작 위치 변경
		pPlayer->m_packet_start += p_size;
		
		// recv한 데이터를 처리를 했는데 아직 처리하지 못한 데이터를 가지고 있을 수도 있다.
		// 처리해야 할 패킷 크기 갱신
		if (pPlayer->m_packet_start < next_recv_ptr)
			p_size = pPlayer->m_packet_start[0];
		else
			break;
	} 

	// 처리하지 못한(남아 있는) 데이터의 시작 위치 
	long long left_data = next_recv_ptr - pPlayer->m_packet_start;

	// Recv 처리 끝 -> 다시 Recv를 해야 한다.
	// 다시 Recv를 하기 전에 링 버퍼의 여유 공간을 확인 후 다 사용했을 경우 초기화
	// 남아있는 공간이 MIN_BUUFER 보다 작으면 남은 데이터를 링버퍼 맨 앞으로 옮겨준다. 
	if ((MAX_BUFFER - (next_recv_ptr - pPlayer->m_recv_over.iocp_buf)) < MIN_BUFFER)
	{
		// 남은 데이터 버퍼를 iocp_buf에 복사
		memcpy(pPlayer->m_recv_over.iocp_buf, pPlayer->m_packet_start, left_data);
		// 처리할 데이터 버퍼 갱신
		pPlayer->m_packet_start = pPlayer->m_recv_over.iocp_buf;
		// 다음 패킷을 받을 버퍼의 시작위치 갱신
		next_recv_ptr = pPlayer->m_packet_start + left_data;
	}

	// 링 버퍼 공간 갱신 여부 확인 후 다음 Recv 준비하기
	DWORD recv_flag = 0;
	pPlayer->m_recv_start = next_recv_ptr;
	pPlayer->m_recv_over.wsa_buf.buf = reinterpret_cast<CHAR*>(next_recv_ptr);
	pPlayer->m_recv_over.wsa_buf.len = MAX_BUFFER - static_cast<int>(next_recv_ptr - pPlayer->m_recv_over.iocp_buf);

	pPlayer->Get_ClientLock().lock();
	if (pPlayer->Get_IsConnected())
		WSARecv(pPlayer->m_sock, &pPlayer->m_recv_over.wsa_buf, 1, NULL, &recv_flag, &pPlayer->m_recv_over.wsa_over, NULL);
	pPlayer->Get_ClientLock().unlock();
}

/*===========================================PLAYER=====================================================*/
void send_packet(int id, void* p)
{
	unsigned char* packet = reinterpret_cast<unsigned char*>(p);

	OVER_EX* send_over = new OVER_EX;

	memcpy(send_over->iocp_buf, packet, packet[0]);
	send_over->op_mode = OP_MODE_SEND;
	send_over->wsa_buf.buf = reinterpret_cast<CHAR*>(send_over->iocp_buf);
	send_over->wsa_buf.len = packet[0];
	ZeroMemory(&send_over->wsa_over, sizeof(send_over->wsa_over));

	CPlayer* pPlayer = static_cast<CPlayer*>(CObjMgr::GetInstance()->Get_GameObject(L"PLAYER", id));

	pPlayer->Get_ClientLock().lock();
	if (pPlayer->Get_IsConnected())
		WSASend(pPlayer->m_sock, &send_over->wsa_buf, 1, NULL, 0, &send_over->wsa_over, NULL);
	pPlayer->Get_ClientLock().unlock();
}

void send_login_ok(int id)
{
	sc_packet_login_ok p;

	CPlayer* pPlayer = static_cast<CPlayer*>(CObjMgr::GetInstance()->Get_GameObject(L"PLAYER", id));

	if (pPlayer == nullptr) return;

	p.size = sizeof(p);
	p.type = SC_PACKET_LOGIN_OK;
	p.id = id;

	p.att = pPlayer->att;
	p.exp = pPlayer->Exp;
	p.hp = pPlayer->Hp;
	p.level = pPlayer->level;
	p.maxExp = pPlayer->maxExp;
	p.maxHp = pPlayer->maxHp;
	p.spd = pPlayer->spd;

	p.posX = pPlayer->m_vPos.x;
	p.posY = pPlayer->m_vPos.y;
	p.posZ = pPlayer->m_vPos.z;

	p.dirX = pPlayer->m_vDir.x;
	p.dirY = pPlayer->m_vDir.y;
	p.dirZ = pPlayer->m_vDir.z;

	send_packet(id, &p);
}

void send_enter_packet(int to_client, int new_id)
{
	sc_packet_enter p;

	CPlayer* pNewPlayer = static_cast<CPlayer*>(CObjMgr::GetInstance()->Get_GameObject(L"PLAYER", new_id));

	p.size = sizeof(p);
	p.type = SC_PACKET_ENTER;
	p.id = new_id;

	pNewPlayer->Get_ClientLock().lock();
	strcpy_s(p.name, pNewPlayer->m_ID);
	pNewPlayer->Get_ClientLock().unlock();
	p.o_type = pNewPlayer->m_type;

	p.posX = pNewPlayer->m_vPos.x;
	p.posY = pNewPlayer->m_vPos.y;
	p.posZ = pNewPlayer->m_vPos.z;

	p.dirX = pNewPlayer->m_vDir.x;
	p.dirY = pNewPlayer->m_vDir.y;
	p.dirZ = pNewPlayer->m_vDir.z;

	send_packet(to_client, &p);
}

void send_leave_packet(int to_client, int leave_id)
{
	sc_packet_leave p;

	p.size = sizeof(p);
	p.type = SC_PACKET_LEAVE;
	p.id = leave_id;

	send_packet(to_client, &p);
}

void send_move_packet(int to_client, int id)
{
	sc_packet_move p;

	CPlayer* pPlayer = static_cast<CPlayer*>(CObjMgr::GetInstance()->Get_GameObject(L"PLAYER", id));

	if (pPlayer == nullptr) return;

	p.size = sizeof(p);
	p.type = SC_PACKET_MOVE;
	p.id = id;

	//p.move_time = pNewPlayer->move_time;

	p.posX = pPlayer->m_vPos.x;
	p.posY = pPlayer->m_vPos.y;
	p.posZ = pPlayer->m_vPos.z;

	p.dirX = pPlayer->m_vDir.x;
	p.dirY = pPlayer->m_vDir.y;
	p.dirZ = pPlayer->m_vDir.z;

	send_packet(to_client, &p);
}

void process_move(int id, cs_packet_move* info)
{
	CPlayer* pPlayer = static_cast<CPlayer*>(CObjMgr::GetInstance()->Get_GameObject(L"PLAYER", id));

	if (pPlayer == nullptr) return;

	/* 해당 플레이어의 원래 위치값 */
	float ori_x, ori_y, ori_z;
	ori_x = pPlayer->m_vPos.x;
	ori_y = pPlayer->m_vPos.y;
	ori_z = pPlayer->m_vPos.z;

	/* 해당 플레이어의 원래 시야 목록 */
	pPlayer->v_lock.lock();
	unordered_set<int> old_viewlist = pPlayer->view_list;
	pPlayer->v_lock.unlock();

	/* 해당 플레이어로부터 받은 변경된 위치값 저장 */
	pPlayer->m_vPos.x = info->posX;
	pPlayer->m_vPos.y = info->posY;
	pPlayer->m_vPos.z = info->posZ;

	pPlayer->m_vDir.x = info->dirX;
	pPlayer->m_vDir.y = info->dirY;
	pPlayer->m_vDir.z = info->dirZ;

	/* 변경된 좌표로 섹터 갱신 */
	CSectorMgr::GetInstance()->Compare_exchange_Sector(id, (int)ori_y, (int)ori_x, (int)(pPlayer->m_vPos.y), (int)(pPlayer->m_vPos.x));


	/* 플레이어 주변의 섹터를 검색 -> 인접한 섹터 내의 객체에게 좌표 전송 */
	unordered_set<pair<int, int>> nearSector;
	nearSector.reserve(5);
	CSectorMgr::GetInstance()->Get_NearSectorIndex(&nearSector, (int)pPlayer->m_vPos.x, (int)pPlayer->m_vPos.y);

	// 해당 플레이어의 갱신된 시야 목록
	unordered_set<int> new_viewlist;

	// 인접 섹터 순회
	for (auto& s : nearSector)
	{
		// 인접 섹터 내의 타 유저들이 있는지 검사
		if (!(CSectorMgr::GetInstance()->Get_SectorList()[s.first][s.second].Get_ObjList().empty()))
		{
			// 타 유저의 서버 번호 추출
			for (auto obj_num : CSectorMgr::GetInstance()->Get_SectorList()[s.first][s.second].Get_ObjList())
			{
				/* 타유저일 경우 처리 */
				if (obj_num == id) continue;
				if (true == CObjMgr::GetInstance()->Is_Player(obj_num))
				{
					CPlayer* pOther = static_cast<CPlayer*>(CObjMgr::GetInstance()->Get_GameObject(L"PLAYER", obj_num));
					
					// 접속한 유저만 시야 목록에 등록한다.
					if (!pOther->Get_IsConnected()) continue;
					
					// 시야 내에 있다면 시야 목록에 등록한다.
					if (CObjMgr::GetInstance()->Is_Near(pPlayer, pOther))
						new_viewlist.insert(obj_num);			
				}
				/* NPC일 경우 처리 */
				else if (true == CObjMgr::GetInstance()->Is_NPC(obj_num))
				{
					CNpc* pNPC = static_cast<CNpc*>(CObjMgr::GetInstance()->Get_GameObject(L"NPC", obj_num));

					// 시야 내에 없다면 시야 목록에 등록X.
					if (CObjMgr::GetInstance()->Is_Near(pPlayer, pNPC))
						new_viewlist.insert(obj_num);				
				}
				/* MONSTER일 경우 처리*/
				else
				{
					int i = 0;
				}
			}
		}
	}

	/* 새로운 시야 목록 내의 객체 처리 */
	for (int server_num : new_viewlist)
	{
		// 플레이어 시야 목록에 새로 들어온 객체 처리 (이전 시야 목록에 없다면)
		if (0 == old_viewlist.count(server_num))
		{
			// 플레이어 시야 목록에 추가
			pPlayer->v_lock.lock();
			pPlayer->view_list.insert(server_num);
			pPlayer->v_lock.unlock();

			// 새로운 타유저의 시야 처리
			if (true == CObjMgr::GetInstance()->Is_Player(server_num))
			{
				// 플레이어('나')에게 새로운 유저 등장 패킷 전송
				send_enter_packet(id, server_num);

				CPlayer* pOther = static_cast<CPlayer*>(CObjMgr::GetInstance()->Get_GameObject(L"PLAYER", server_num));

				// 타 유저의 시야 목록 처리
				pOther->v_lock.lock();
				// 타 유저의 시야 목록에 '나'가 새로 들어온 경우
				if (0 == pOther->view_list.count(id))
				{
					pOther->view_list.insert(id);
					pOther->v_lock.unlock();
					send_enter_packet(server_num, id);
				}
				else
				{
					pOther->v_lock.unlock();
					send_move_packet(server_num, id);
				}
			}
			// 새로 시야에 들어온 NPC일 경우 처리
			else if (true == CObjMgr::GetInstance()->Is_NPC(server_num))
			{
				// 플레이어('나')에게 NPC등장 패킷 전송
				send_NPC_enter_packet(id, server_num);
			}
			// 새로 시야에 들어온 MONSTER일 경우 처리
			else
			{
				int i = 0;
			}
		}
		// 플레이어 시야 목록에 계속 있는 객체 처리
		else
		{
			// 타 유저 처리
			if (true == CObjMgr::GetInstance()->Is_Player(server_num))
			{
				CPlayer* pOther = static_cast<CPlayer*>(CObjMgr::GetInstance()->Get_GameObject(L"PLAYER", server_num));

				// 타 유저의 시야 목록 처리
				pOther->v_lock.lock();
				// 타 유저의 시야 목록에 '나'가 계속 있는 경우
				if (0 != pOther->view_list.count(id))
				{
					pOther->v_lock.unlock();
					send_move_packet(server_num, id);
				}
				// 타 유저의 시야 목록에 '나'가 새로 들어온 경우
				else
				{
					pOther->view_list.insert(id);
					pOther->v_lock.unlock();
					send_enter_packet(server_num, id);
				}
			}
		} // 상대방에게 나의 스탯 정보 갱신 .... 나중에
	}

	/* 이전 시야 목록에서 사라진 객체 처리 */
	for (int s_num : old_viewlist)
	{
		// 갱신된 시야 목록에 없는 객체일 경우
		if (0 == new_viewlist.count(s_num))
		{
			pPlayer->v_lock.lock();
			pPlayer->view_list.erase(s_num);
			pPlayer->v_lock.unlock();
			send_leave_packet(id, s_num);

			// 타 유저 처리
			if (true == CObjMgr::GetInstance()->Is_Player(s_num))
			{
				CPlayer* pOther = static_cast<CPlayer*>(CObjMgr::GetInstance()->Get_GameObject(L"PLAYER", s_num));

				// 타 유저의 시야 목록 처리
				pOther->v_lock.lock();

				// 타 유저의 시야에 '나'가 있다면 삭제
				if (0 != pOther->view_list.count(id))
				{
					pOther->view_list.erase(id);
					pOther->v_lock.unlock();
					send_leave_packet(s_num, id);
				}
				else pOther->v_lock.unlock();
			}
		}
	}
}

/*============================================NPC======================================================*/
void send_NPC_enter_packet(int to_client, int new_id)
{
	sc_packet_enter p;

	CNpc* pNewPlayer = static_cast<CNpc*>(CObjMgr::GetInstance()->Get_GameObject(L"NPC", new_id));

	p.size = sizeof(p);
	p.type = SC_PACKET_ENTER;
	p.id = new_id;

	pNewPlayer->Get_ClientLock().lock();
	strcpy_s(p.name, pNewPlayer->m_ID);
	pNewPlayer->Get_ClientLock().unlock();
	p.o_type = pNewPlayer->m_type;

	p.posX = pNewPlayer->m_vPos.x;
	p.posY = pNewPlayer->m_vPos.y;
	p.posZ = pNewPlayer->m_vPos.z;

	p.dirX = pNewPlayer->m_vDir.x;
	p.dirY = pNewPlayer->m_vDir.y;
	p.dirZ = pNewPlayer->m_vDir.z;

	send_packet(to_client, &p);
}
