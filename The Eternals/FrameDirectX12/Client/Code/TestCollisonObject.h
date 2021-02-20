#pragma once
#include "GameObject.h"

namespace Engine
{
	class CShaderColorInstancing;
	class CColliderSphere;
	class CColliderBox;
}

class CTestCollisonObject final : public Engine::CGameObject
{
private:
	explicit CTestCollisonObject(ID3D12Device* pGraphicDevice, ID3D12GraphicsCommandList* pCommandList);
	explicit CTestCollisonObject(const CTestCollisonObject& rhs);
	virtual ~CTestCollisonObject() = default;

public:
	// CGameObject��(��) ���� ��ӵ�
	virtual HRESULT	Ready_GameObject(const _vec3 & vScale,
									 const _vec3 & vAngle,
									 const _vec3 & vPos);
	virtual HRESULT	LateInit_GameObject();
	virtual _int	Update_GameObject(const _float& fTimeDelta);
	virtual _int	LateUpdate_GameObject(const _float& fTimeDelta);
	virtual void	Render_GameObject(const _float& fTimeDelta);

private:
	virtual HRESULT Add_Component();

private:
	/*__________________________________________________________________________________________________________
	[ Component ]
	____________________________________________________________________________________________________________*/
	Engine::CShaderColorInstancing* m_pShaderColorInstancing = nullptr;
	Engine::CColliderSphere*		m_pColliderSphereCom     = nullptr;
	Engine::CColliderBox*			m_pColliderBoxCom        = nullptr;

public:
	static Engine::CGameObject* Create(ID3D12Device* pGraphicDevice,
									   ID3D12GraphicsCommandList* pCommandList,
									   const _vec3 & vScale,
									   const _vec3 & vAngle,
									   const _vec3 & vPos);
private:
	virtual void Free();

};
