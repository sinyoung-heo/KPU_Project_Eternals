#pragma once
#include "Include.h"
#include "GameObject.h"

namespace Engine
{
	class CTrailTex;
	class CTexture;
	class CShaderTexture;
}

// const _uint TRAIL_SIZE = 25;


class CEffectTrail : public Engine::CGameObject
{
private:
	explicit CEffectTrail(ID3D12Device* pGraphicDevice, ID3D12GraphicsCommandList* pCommandList);
	virtual ~CEffectTrail() = default;

public:
	const _bool& Get_IsRenderTrail() { return m_bIsRender; }

	void Set_IsRenderTrail(const _bool& bIsRenderTrail) { m_bIsRender = bIsRenderTrail;  }

	// CGameObject��(��) ���� ��ӵ�
	virtual HRESULT	Ready_GameObject(wstring wstrTextureTag,
									 const _uint& uiTexIdx);
	virtual HRESULT	LateInit_GameObject();
	virtual _int	Update_GameObject(const _float& fTimeDelta);
	virtual _int	LateUpdate_GameObject(const _float& fTimeDelta);
	virtual void	Render_GameObject(const _float& fTimeDelta);

	void SetUp_TrailByCatmullRom(_vec3* vMin, _vec3* vMax);

private:
	virtual HRESULT Add_Component(wstring wstrTextureTag);
private:
	void Set_ConstantTable();
	void SetUp_TrailAlpha(const _float& fTimeDelta);

private:
	/*__________________________________________________________________________________________________________
	[ Component ]
	____________________________________________________________________________________________________________*/
	Engine::CTrailTex*		m_pBufferCom  = nullptr;
	Engine::CTexture*		m_pTextureCom = nullptr;
	Engine::CShaderTexture*	m_pShaderCom  = nullptr;

	/*__________________________________________________________________________________________________________
	[ Value ]
	____________________________________________________________________________________________________________*/
	array<_vec3, Engine::TRAIL_SIZE - 1> m_arrMax;
	wstring	m_wstrTextureTag = L"";
	_uint	m_uiTexIdx		 = 0;
	_float	m_fAlpha		 = 0.0f;
	_bool	m_bIsRender      = false;


public:
	static CEffectTrail* Create(ID3D12Device* pGraphicDevice, ID3D12GraphicsCommandList* pCommandList,
								wstring wstrTextureTag,
								const _uint& uiTexIdx);
private:
	virtual void Free();
};
