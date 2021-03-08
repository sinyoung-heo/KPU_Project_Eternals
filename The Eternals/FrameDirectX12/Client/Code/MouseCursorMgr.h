#pragma once
#include "Include.h"
#include "Base.h"

namespace Engine
{
	class CGameObject;
}

const int MOUSE_HIDE = INT_MIN;

class CMouseCursorMgr : public Engine::CBase
{
	DECLARE_SINGLETON(CMouseCursorMgr)

private:
	explicit CMouseCursorMgr();
	virtual ~CMouseCursorMgr() = default;

public:
	static POINT	Get_CursorPoint();
	const _bool&	Get_IsActiveMouse()							{ return m_bIsActiveMouse; }
	void			Set_IsActiveMouse(const _bool& bIsActive)	{ m_bIsActiveMouse = bIsActive; }
	void			Reset_MouseCursor()							{ m_pMouseCursor = nullptr; }
public:
	HRESULT			Ready_MouseCursorMgr();
	void			Update_MouseCursorMgr(const _float& fTimeDelta);
public:
	void			Is_ActiveMouse() { m_bIsActiveMouse = !m_bIsActiveMouse; }

private:
	Engine::CGameObject*	m_pMouseCursor   = nullptr;
	_bool					m_bIsActiveMouse = true;
	_bool					m_bIsResetMouse  = false;

private:
	virtual void Free();
};

