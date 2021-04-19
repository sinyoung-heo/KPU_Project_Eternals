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