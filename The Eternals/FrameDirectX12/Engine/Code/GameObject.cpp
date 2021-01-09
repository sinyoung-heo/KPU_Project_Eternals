#include "GameObject.h"

#include "GraphicDevice.h"
#include "ObjectMgr.h"
#include "Renderer.h"
#include "ComponentMgr.h"
#include "Renderer.h"
#include "FrustumMgr.h"

USING(Engine)

CGameObject::CGameObject(ID3D12Device* pGraphicDevice, ID3D12GraphicsCommandList* pCommandList)
	: m_pGraphicDevice(pGraphicDevice)
	, m_pCommandList(pCommandList)
	, m_pObjectMgr(CObjectMgr::Get_Instance())
	, m_pRenderer(CRenderer::Get_Instance())
	, m_pComponentMgr(CComponentMgr::Get_Instance())
	, m_pFrustumMgr(CFrustumMgr::Get_Instance())
{
	ZeroMemory(&m_tShadowInfo, sizeof(CB_SHADOW_DESC));
}

CGameObject::CGameObject(const CGameObject & rhs)
	: m_pGraphicDevice(rhs.m_pGraphicDevice)
	, m_pCommandList(rhs.m_pCommandList)
	, m_pRenderer(rhs.m_pRenderer)
	, m_pObjectMgr(rhs.m_pObjectMgr)
	, m_pComponentMgr(rhs.m_pComponentMgr)
	, m_pFrustumMgr(rhs.m_pFrustumMgr)

	, m_bIsDead(rhs.m_bIsDead)
	, m_bIsLateInit(rhs.m_bIsLateInit)
	, m_fViewZ(rhs.m_fViewZ)
	, m_UIDepth(rhs.m_UIDepth)
	, m_tShadowInfo(rhs.m_tShadowInfo)
{
}



CComponent * CGameObject::Get_Component(wstring wstrComponentTag, COMPONENTID eID)
{
	CComponent* pComponent = Find_Component(wstrComponentTag, eID);
	NULL_CHECK_RETURN(pComponent, nullptr);

	return pComponent;
}

HRESULT CGameObject::Ready_GameObjectPrototype()
{
	return S_OK;
}

HRESULT CGameObject::Ready_GameObject(const _bool & bIsCreate_Transform, const _bool & bIsCreate_Info)
{
	if (bIsCreate_Transform)
	{
		m_pTransCom = Engine::CTransform::Create();
		NULL_CHECK_RETURN(m_pTransCom, E_FAIL);
		m_mapComponent[ID_DYNAMIC].emplace(L"Com_Transform", m_pTransCom);
	}

	if (bIsCreate_Info)
	{
		m_pInfoCom = Engine::CInfo::Create();;
		NULL_CHECK_RETURN(m_pInfoCom, E_FAIL);
		m_mapComponent[ID_STATIC].emplace(L"Com_Info", m_pInfoCom);
	}

	return S_OK;
}

HRESULT CGameObject::LateInit_GameObject()
{
	if (!m_bIsLateInit)
	{
		FAILED_CHECK_RETURN(this->LateInit_GameObject(), E_FAIL);
		m_bIsLateInit = true;
	}

	return S_OK;
}

_int CGameObject::Update_GameObject(const _float & fTimeDelta)
{
	for (auto& iter : m_mapComponent[ID_DYNAMIC])
		iter.second->Update_Component(fTimeDelta);


	return NO_EVENT;
}

_int CGameObject::LateUpdate_GameObject(const _float & fTimeDelta)
{
	return NO_EVENT;
}

void CGameObject::Render_GameObject(const _float & fTimeDelta)
{
}

void CGameObject::Render_ShadowDepth(const _float & fTimeDelta)
{
}

void CGameObject::Render_GameObject(const _float& fTimeDelta, ID3D12GraphicsCommandList * pCommandList, const _int& iContextIdx)
{
}

void CGameObject::Render_ShadowDepth(const _float& fTimeDelta, ID3D12GraphicsCommandList * pCommandList, const _int& iContextIdx)
{
}

HRESULT CGameObject::Add_Component()
{
	return S_OK;
}

void CGameObject::SetUp_BillboardMatrix()
{
	if (nullptr != m_pTransCom)
	{
		_matrix* pmatView = CGraphicDevice::Get_Instance()->Get_Transform(MATRIXID::VIEW);
		if (nullptr == pmatView)
			return;

		_matrix matBillboard = (*pmatView);
		memset(&matBillboard._41, 0, sizeof(_vec3));

		matBillboard = MATRIX_INVERSE(matBillboard);

		/*__________________________________________________________________________________________________________
		[ GameObject�� Scale�� �ݿ�. ]
		____________________________________________________________________________________________________________*/
		_float fScale[3] = { m_pTransCom->m_vScale.x, m_pTransCom->m_vScale.y, m_pTransCom->m_vScale.z };

		for (_int i = 0; i < 3; ++i)
		{
			for (_int j = 0; j < 4; ++j)
			{
				matBillboard(i, j) *= fScale[i];
			}
		}

		/*__________________________________________________________________________________________________________
		[ Scale + Rotate �Է� ]
		____________________________________________________________________________________________________________*/
		memcpy(&m_pTransCom->m_matWorld._11, &matBillboard._11, sizeof(_vec3));
		memcpy(&m_pTransCom->m_matWorld._21, &matBillboard._21, sizeof(_vec3));
		memcpy(&m_pTransCom->m_matWorld._31, &matBillboard._31, sizeof(_vec3));

	}

}

void CGameObject::SetUp_ShadowDepth(_vec3 & vLightEye, 
									_vec3 & vLightAt,
									_vec3 & vLightDir)
{
	// LightDir
	vLightDir.y *= -1.5f;
	vLightDir.x *= -1.0f;

	// LightView
	_vec3 vEye						= vLightEye + (vLightDir) * 1000.0f;
	_vec3 vAt						= vLightAt;
	m_tShadowInfo.vLightPosition	= _vec4(vEye, 1.0f);
	m_tShadowInfo.matLightView		= XMMatrixLookAtLH(vEye.Get_XMVECTOR(),
													   vAt.Get_XMVECTOR(),
													   _vec3(0.0f, 1.0f, 0.0f).Get_XMVECTOR());
	// LightProj
	m_tShadowInfo.fLightPorjFar		= 10000.0f;
	m_tShadowInfo.matLightProj		= XMMatrixPerspectiveFovLH(XMConvertToRadians(15.0f),
															   1.0f,
															   1.0f, 
															   m_tShadowInfo.fLightPorjFar);
}

void CGameObject::Compute_ViewZ(_vec4& vPosInWorld)
{
	_matrix*	pmatView	= CGraphicDevice::Get_Instance()->Get_Transform(MATRIXID::VIEW);
	_matrix		matViewInv	= XMMatrixInverse(nullptr, *pmatView);

	_vec4 vCamPositionInWorld;
	memcpy(&vCamPositionInWorld, &matViewInv._41, sizeof(_vec4));

	_vec4 vDir = (vPosInWorld) - vCamPositionInWorld;

	m_fViewZ = vDir.Get_Length();
}


CComponent * CGameObject::Find_Component(wstring wstrComponentTag, const COMPONENTID & eID)
{
	auto iter_find = m_mapComponent[eID].find(wstrComponentTag);

	if (iter_find == m_mapComponent[eID].end())
		return nullptr;

	return iter_find->second;
}


CGameObject * CGameObject::Clone_GameObject()
{
	return nullptr;
}

void CGameObject::Free()
{
	for (_uint i = 0; i < ID_END; ++i)
	{
		for_each(m_mapComponent[i].begin(), m_mapComponent[i].end(), CDeleteMap());
		m_mapComponent[i].clear();
	}
}