#pragma once

#include "Include.h"
#include "GameObject.h"

/*____________________________________________________________________
Player Direction
______________________________________________________________________*/
#define	FRONT		0.f
#define	RIGHT		90.f
#define	RIGHT_UP	45.f
#define	RIGHT_DOWN	135.f
#define	LEFT		270.f
#define	LEFT_UP		315.f
#define	LEFT_DOWN	225.f
#define	BACK		180.f

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

class CTestPlayer : public Engine::CGameObject
{
private:
	explicit CTestPlayer(ID3D12Device* pGraphicDevice, ID3D12GraphicsCommandList* pCommandList);
	explicit CTestPlayer(const CTestPlayer& rhs);
	virtual ~CTestPlayer() = default;

public:
	// CGameObject을(를) 통해 상속됨
	virtual HRESULT	Ready_GameObject(wstring wstrMeshTag,
		const _vec3& vScale,
		const _vec3& vAngle,
		const _vec3& vPos);
	virtual HRESULT	LateInit_GameObject();
	virtual _int	Update_GameObject(const _float& fTimeDelta);
	virtual _int	LateUpdate_GameObject(const _float& fTimeDelta);

	// SingleThread Rendering.
	virtual void	Render_GameObject(const _float& fTimeDelta);
	virtual void	Render_ShadowDepth(const _float& fTimeDelta);

	// MultiThread Rendering
	virtual void	Render_GameObject(const _float& fTimeDelta, ID3D12GraphicsCommandList* pCommandList, const _int& iContextIdx);
	virtual void	Render_ShadowDepth(const _float& fTimeDelta, ID3D12GraphicsCommandList* pCommandList, const _int& iContextIdx);

public:
	void			Set_DeadReckoning(const _vec3& vPos1, const _vec3& vPos2, const _vec3& vPos3, const _vec3& vPos4);
private:
	virtual HRESULT Add_Component(wstring wstrMeshTag);
	void			Set_ConstantTable();
	void			Set_ConstantTableShadowDepth();

	void			Key_Input(const _float& fTimeDelta);
	void			Send_Player_Move();

public:
	static Engine::CGameObject* Create(ID3D12Device* pGraphicDevice,
		ID3D12GraphicsCommandList* pCommandList,
		wstring wstrMeshTag,
		const _vec3& vScale,
		const _vec3& vAngle,
		const _vec3& vPos);
private:
	virtual void Free();

private:
	/*__________________________________________________________________________________________________________
	[ Component ]
	____________________________________________________________________________________________________________*/
	Engine::CMesh* m_pMeshCom = nullptr;
	Engine::CShaderMesh* m_pShaderCom = nullptr;
	Engine::CShaderShadow* m_pShadowCom = nullptr;
	Engine::CColliderSphere* m_pColliderSphereCom = nullptr;
	Engine::CColliderBox* m_pColliderBoxCom = nullptr;
	Engine::CNaviMesh* m_pNaviMeshCom = nullptr;

	/*__________________________________________________________________________________________________________
	[ Value ]
	____________________________________________________________________________________________________________*/
	CDynamicCamera* m_pDynamicCamera = nullptr;
	wstring			m_wstrMeshTag = L"";

	/* Server */
	bool			m_bIsKeyUp = false;
	float			m_fBazierSpeed = 0.f;
	MVKEY			m_eKeyState = MVKEY::K_END;

	/*__________________________________________________________________________________________________________
	[ Animation Frame ]
	____________________________________________________________________________________________________________*/
	_uint			m_uiAnimIdx = 0;	// 현재 애니메이션 Index
	_uint			m_ui3DMax_NumFrame = 0;	// 3DMax에서 애니메이션의 총 Frame 개수
	_uint			m_ui3DMax_CurFrame = 0;	// 3DMAx에서 현재 애니메이션의 Frame 위치

	/*__________________________________________________________________________________________________________
	[ Font ]
	____________________________________________________________________________________________________________*/
	Engine::CFont*	m_pFont = nullptr;
	wstring			m_wstrText = L"";
	_tchar			m_szText[MAX_STR] = L"";
};

