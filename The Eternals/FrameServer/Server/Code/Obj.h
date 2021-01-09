#pragma once
class CObj
{
public:
	CObj() = default;
	virtual ~CObj() = default;

public:
	virtual DWORD Release();

protected:
	/*=============�ý��� ������==============*/
	int move_time;
	SOCKET	m_sock; // player
	OVER_EX	m_recv_over;  // player 
	unsigned char* m_packet_start; // player
	unsigned char* m_recv_start; // player

	/*=============���� ������===============*/
	bool m_bIsConnect; // all
	bool m_bIsDead; //all
	char m_ID[MAX_ID_LEN]; // all
	char m_type; // all
	int	level; // player
	int Hp, maxHp; // player, monster
	int Exp, maxExp; // player
	int att; // player monster
	float spd; // player monster
	_vec3 m_vPos; // all
	_vec3 m_vDir; // all

	mutex c_lock;  // all

	unordered_set<int> view_list; // player
	mutex v_lock; // player 
	atomic<STATUS> m_status; // all
};
