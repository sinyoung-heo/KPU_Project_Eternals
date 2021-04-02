#pragma once
#include "Include.h"
#include "GameObject.h"
#include "GladiatorAnimation.h"
#include "PCWeaponTwoHand.h"

namespace GladiatorConst
{
	const _float MAX_SPEED       = 5.0f;
	const _float MIN_SPEED       = 0.0f;
	const _float MOVE_STOP_SPEED = 3.0f;

	enum COMBOCNT { COMBOCNT_0, COMBOCNT_1, COMBOCNT_2, COMBOCNT_3, COMBO_END };
}

namespace Engine
{
	class CMesh;
	class CShaderMesh;
	class CShaderShadow;
	class CColliderSphere;
	class CColliderBox;
	class CFont;
	class CNaviMesh;
}

class CDynamicCamera;

class CPCGladiator : public Engine::CGameObject
{
private:
	explicit CPCGladiator(ID3D12Device* pGraphicDevice, ID3D12GraphicsCommandList* pCommandList);
	virtual ~CPCGladiator() = default;

public:
	// CGameObject을(를) 통해 상속됨
	virtual HRESULT	Ready_GameObject(wstring wstrMeshTag,
									 wstring wstrNaviMeshTag,
									 const _vec3& vScale,
									 const _vec3& vAngle,
									 const _vec3& vPos,
									 const char& chWeaponType);
	virtual HRESULT	LateInit_GameObject();
	virtual _int	Update_GameObject(const _float& fTimeDelta);
	virtual _int	LateUpdate_GameObject(const _float& fTimeDelta);
	virtual void	Send_PacketToServer();
	// MultiThread Rendering
	virtual void	Render_GameObject(const _float& fTimeDelta, ID3D12GraphicsCommandList* pCommandList, const _int& iContextIdx);
	virtual void	Render_ShadowDepth(const _float& fTimeDelta, ID3D12GraphicsCommandList* pCommandList, const _int& iContextIdx);
private:
	virtual HRESULT Add_Component(wstring wstrMeshTag, wstring wstrNaviMeshTag);
	HRESULT			SetUp_PCWeapon();
	void			Set_ConstantTable();
	void			Set_ConstantTableShadowDepth();

	// KeyInput
	void Key_Input(const _float& fTimeDelta);
	void KeyInput_Move(const _float& fTimeDelta);
	void KeyInput_Attack(const _float& fTimeDelta);
	void KeyInput_StanceChange(const _float& fTimeDelta);
	void KeyInput_ComboAttack(const _float& fTimeDelta);
	void SetUp_ComboAttackAnimation();
	void SetUp_FromComboAttackToAttackWait(const _float& fTimeDelta);
	void Move_OnNaviMesh(const _float& fTimeDelta);
	void Send_Player_Move();
	bool Is_Change_CamDirection();

	void SetUp_RunMoveSpeed(const _float& fTimeDelta);
	void SetUp_RunAnimation();
	void SetUp_RunToIdleAnimation(const _float& fTimeDelta);
	void SetUp_PlayerStance_FromAttackToNoneAttack();
	void SetUp_PlayerStance_FromNoneAttackToAttack();
	void Change_PlayerStance(const _float& fTimeDelta);
	void Ready_AngleInterpolationValue(const _float& fEndAngle);
	void SetUp_AngleInterpolation(const _float& fTimeDelta);

private:
	/*__________________________________________________________________________________________________________
	[ Component ]
	____________________________________________________________________________________________________________*/
	Engine::CMesh*				m_pMeshCom           = nullptr;
	Engine::CShaderMesh*		m_pShaderCom         = nullptr;
	Engine::CShaderShadow*		m_pShadowCom         = nullptr;
	Engine::CColliderSphere*	m_pColliderSphereCom = nullptr;
	Engine::CColliderBox*		m_pColliderBoxCom    = nullptr;
	Engine::CNaviMesh*			m_pNaviMeshCom       = nullptr;

	/*__________________________________________________________________________________________________________
	[ Manager ]
	____________________________________________________________________________________________________________*/
	CPacketMgr*		m_pPacketMgr  = nullptr;
	CServerMath*	m_pServerMath = nullptr;

	/*__________________________________________________________________________________________________________
	[ Value ]
	____________________________________________________________________________________________________________*/
	CPCWeaponTwoHand*	m_pWeapon        = nullptr;
	CDynamicCamera*		m_pDynamicCamera = nullptr;
	wstring				m_wstrMeshTag    = L"";

	// Speed Linear Interpolation
	_float m_fSpeedInterpolationRatio = 0.0f;

	// Server
	_bool			m_bIsKeyDown   = false;
	_bool			m_bIsSameDir   = false;
	_bool			m_bIsAttack    = false;
	_float			m_fBazierSpeed = 0.f;
	MVKEY			m_eKeyState    = MVKEY::K_END;
	_float			m_fPreAngle    = 0.f;
	char			m_chWeaponType = -1;

	/*__________________________________________________________________________________________________________
	[ Animation ]
	____________________________________________________________________________________________________________*/
	_uint m_uiAnimIdx        = 0;	// 현재 애니메이션 Index
	_uint m_ui3DMax_NumFrame = 0;	// 3DMax에서 애니메이션의 총 Frame 개수
	_uint m_ui3DMax_CurFrame = 0;	// 3DMAx에서 현재 애니메이션의 Frame 위치

	Gladiator::STANCE	m_eStance                 = Gladiator::STANCE_END;
	_bool				m_bIsCompleteStanceChange = true;
	_uint				m_uiComoboCnt             = 0;

	/*__________________________________________________________________________________________________________
	[ Font ]
	____________________________________________________________________________________________________________*/
	Engine::CFont*	m_pFont           = nullptr;
	wstring			m_wstrText        = L"";
	_tchar			m_szText[MAX_STR] = L"";

public:
	static Engine::CGameObject* Create(ID3D12Device* pGraphicDevice,
									   ID3D12GraphicsCommandList* pCommandList,
									   wstring wstrMeshTag,
									   wstring wstrNaviMeshTag,
									   const _vec3& vScale,
									   const _vec3& vAngle,
									   const _vec3& vPos,
									   const char& chWeaponType);
private:
	virtual void Free();
};

