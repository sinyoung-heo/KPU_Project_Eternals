#pragma once
#include "Include.h"
#include "Scene.h"
#include "Loading.h"

namespace Engine
{
	class CFont;
}

class CScene_Logo final : public Engine::CScene
{
private:
	explicit CScene_Logo(ID3D12Device* pGraphicDevice, ID3D12GraphicsCommandList* pCommandList);
	virtual ~CScene_Logo() = default;

public:
	void Set_IsSceneChage(const _bool& bIsSceneChange) { m_bIsSceneChange = bIsSceneChange; }

	virtual HRESULT Ready_Scene();
	virtual _int	Update_Scene(const _float& fTimeDelta);
	virtual _int	LateUpdate_Scene(const _float& fTimeDelta);
	virtual HRESULT	Render_Scene(const _float& fTimeDelta, const Engine::RENDERID& eID = Engine::RENDERID::MULTI_THREAD);

private:
	HRESULT			Ready_LayerEnvironment(wstring wstrLayerTag);
	HRESULT			Ready_LayerCamera(wstring wstrLayerTag);
	HRESULT			Ready_LayerGameObject(wstring wstrLayerTag);
	HRESULT			Ready_LayerUI(wstring wstrLayerTag);
	HRESULT			Ready_LayerFont(wstring wstrLayerTag);
	HRESULT			SetUp_MaxLoadingCount();

private:
	CLoading*				m_pLoading				= nullptr;
	Engine::CGameObject*	m_pLoadingProgress		= nullptr;

	_bool m_bIsReadyMouseCursorMgr = false;
	_bool m_bIsSceneChange         = false;
	_bool m_bIsCreateFadeInOut	   = false;

	/*__________________________________________________________________________________________________________
	[ Loading Text ]
	____________________________________________________________________________________________________________*/
	Engine::CFont*	m_pFont_LoadingStr		= nullptr;
	_tchar			m_szLoadingStr[MIN_STR]	= L"";
	_int			m_iMaxFileCount			= 0;

public:
	static CScene_Logo*	Create(ID3D12Device* pGraphicDevice, ID3D12GraphicsCommandList* pCommandList);

private:
	virtual void		Free();
};

