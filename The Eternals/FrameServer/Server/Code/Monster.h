#pragma once
#include "Obj.h"
class CMonster :
    public CObj
{
public:
	CMonster();
	virtual ~CMonster();

public:
	void Set_Stop_Attack();
	void Set_Start_Attack();

	int	 Update_Monster(const float& fTimeDelta);

private:
	void Change_Animation(const float& fTimeDelta);

	void Move_ComeBack(const float& fTimeDelta);		// MOVE PROCESS
	void Move_NormalMonster(const float& fTimeDelta);	// MOVE PROCESS
	void Move_ChaseMonster(const float& fTimeDelta);	// MOVE PROCESS
	void Attack_Monster(const float& fTimeDelta);		// ATTACK PROCESS

	void Change_AttackMode();							// STATUS == ATTACK

public:
	/* SEND PACKET */
	void send_Monster_enter_packet(int to_client);
	void send_Monster_move_packet(int to_client);		
	void send_Monster_NormalAttack(int to_client);

	virtual DWORD Release();

public:
	char			m_monNum;
	int				Hp				= 0;
	int				maxHp			= 0;
	int				Exp				= 0;
	int				att				= 0;
	float			spd				= 0;
	bool			m_bIsComeBack	= false;
	volatile bool	m_bIsAttack		= false;

	int		targetNum				= -1;
	int		m_AnimIdx				= 0;
	_vec3   m_vOriPos				= _vec3(0.f);
};

