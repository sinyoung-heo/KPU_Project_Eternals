#pragma once

enum MVKEY
{
	K_FRONT, K_BACK, K_RIGHT, K_RIGHT_UP, K_RIGHT_DOWN, K_LEFT, K_LEFT_UP, K_LEFT_DOWN, K_END
};

enum SHADOW_TYPE 
{ 
	SHADOW_TYPE_ALL, 
	SHADOW_TYPE_PLAYER, 
	SHADOW_TYPE_END 
};

enum EVENT_TYPE 
{ 
	FADE_IN, 
	FADE_OUT,
	SCENE_CHANGE_FADEIN_FADEOUT,
	SCENE_CHANGE_FADEOUT_FADEIN,
	SCENE_CAHNGE_LOGO_STAGE,
	EVENT_END 
};

enum MONSTER
{
	CRAB,
	COLDER_A,
	MONKEY,
	DROWNED_SAILOR,
	GIANT_BEETLE,
	GIANT_MONKEY,
	CRAFTY_ARACCHNE,
	MONSTER_END
};

enum NPC
{
	NPC_END
};

enum ARROW_TYPE
{
	ARROW_DEFAULT,
	ARROW_FALL,
	ARROW_CHARGE,
	ARROW_TYPE_END
};

enum ARROW_POOL_TYPE
{
	ARROW_POOL_ICE,
	ARROW_POOL_FIRE,
	ARROW_POOL_LIGHTNING,
	ARROW_POOL_END
};

enum PARTY_REQUEST_STATE 
{ 
	REQUEST_PARTY_INVITE, 
	REQUEST_PARTY_JOIN, 
	PARTY_STATE_END 
};

enum PARTY_SYSTEM_MESSAGE
{
	ENTER_PARTY_MEMBER,		// 파티원 입장
	LEAVE_PARTY_MEMBER,		// 파티원 탈퇴
	ALREADY_JOIN_IN_PARTY,	// 내가 파티에 가입중.
	SELECT_JOIN_IN_PARYY,	// 상대방이 파티에 가입 중.
	REJECT_PARTY_REQUEST,	// 파티 가입 or 참여 거절.
	PARTY_MESSAGE_END
};

enum CHATTING_CURSOR_MOVE
{
	MOVE_RIGHT,
	MOVE_LEFT,
	MOVE_ORIGIN,
	MOVE_END
};

enum STORE_STATE
{
	STORE_WEAPON,
	STORE_ARMOR,
	STORE_POTION,
	STORE_STATE_END
};

enum ITEM_GRADE
{
	ITEM_D,
	ITEM_C,
	ITEM_B,
	ITEM_A,
	ITEM_S,
	ITEM_GRADE_END
};