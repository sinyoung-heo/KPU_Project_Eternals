#include "stdafx.h"
#include "IceStorm.h"
#include "GraphicDevice.h"
#include "DirectInput.h"
#include "ObjectMgr.h"
#include "LightMgr.h"
#include "DynamicCamera.h"
#include "RenderTarget.h"
#include "TimeMgr.h"
#include "DescriptorHeapMgr.h"
#include "SnowParticle.h"
#include "ParticleEffect.h"
CIceStorm::CIceStorm(ID3D12Device * pGraphicDevice, ID3D12GraphicsCommandList * pCommandList)
	: Engine::CGameObject(pGraphicDevice, pCommandList)
{
}


HRESULT CIceStorm::Ready_GameObject(wstring wstrMeshTag,
											 const _vec3 & vScale,
											 const _vec3 & vAngle, 
											 const _vec3 & vPos ,const float& fRadius,const float & theta)
{
	Engine::FAILED_CHECK_RETURN(Engine::CGameObject::Ready_GameObject(), E_FAIL);
	Engine::FAILED_CHECK_RETURN(Add_Component(wstrMeshTag), E_FAIL);

	m_wstrMeshTag = wstrMeshTag;
	m_pTransCom->m_vScale	= vScale;
	m_pTransCom->m_vAngle	= vAngle;
	m_pTransCom->m_vPos = vPos;
	return S_OK;
}

HRESULT CIceStorm::LateInit_GameObject()
{
	m_pShaderCom->SetUp_ShaderConstantBuffer();
	m_pDescriptorHeaps = Engine::CDescriptorHeapMgr::Get_Instance()->Find_DescriptorHeap(L"EffectPublic");
	m_pCrossFilterShaderCom->SetUp_ShaderConstantBuffer((_uint)(m_pMeshCom->Get_DiffTexture().size()));
	return S_OK;
}

_int CIceStorm::Update_GameObject(const _float & fTimeDelta)
{
	if (m_pTransCom->m_vScale.x < m_fLimitScale)
	{
		m_pTransCom->m_vScale.x += (Engine::CTimerMgr::Get_Instance()->Get_TimeDelta(L"Timer_TimeDelta")) * 0.3f;
		m_pTransCom->m_vScale.y += (Engine::CTimerMgr::Get_Instance()->Get_TimeDelta(L"Timer_TimeDelta")) * 0.3f;
		m_pTransCom->m_vScale.z += (Engine::CTimerMgr::Get_Instance()->Get_TimeDelta(L"Timer_TimeDelta")) * 0.3f;
	}
	Engine::FAILED_CHECK_RETURN(Engine::CGameObject::LateInit_GameObject(), E_FAIL);

	m_fLifeTime += (Engine::CTimerMgr::Get_Instance()->Get_TimeDelta(L"Timer_TimeDelta"));
	
	if (m_fLifeTime > 6.f)
	{
		m_bIsReturn = true;
	}

	if (m_bIsReturn)
	{
		Return_Instance(CInstancePoolMgr::Get_Instance()->Get_Effect_IceStormEffect(), m_uiInstanceIdx);
		return RETURN_OBJ;
	}
	if (m_bIsDead)
		return DEAD_OBJ;

	/*__________________________________________________________________________________________________________
	[ Renderer - Add Render Group ]
	____________________________________________________________________________________________________________*/
	Engine::FAILED_CHECK_RETURN(m_pRenderer->Add_Renderer(Engine::CRenderer::RENDER_MAGICCIRCLE, this), -1);
	Engine::FAILED_CHECK_RETURN(m_pRenderer->Add_Renderer(Engine::CRenderer::RENDER_CROSSFILTER, this), -1);

	/*____________________________________________________________________
	TransCom - Update WorldMatrix.
	______________________________________________________________________*/

	_vec4 vPosInWorld = _vec4(m_pTransCom->m_vPos, 1.0f);
	Engine::CGameObject::Compute_ViewZ(vPosInWorld);

	Engine::CGameObject::Update_GameObject(fTimeDelta);

	return NO_EVENT;
}

_int CIceStorm::LateUpdate_GameObject(const _float & fTimeDelta)
{
	Engine::NULL_CHECK_RETURN(m_pRenderer, -1);


	return NO_EVENT;
}


void CIceStorm::Render_GameObject(const _float& fTimeDelta)
{
	Set_ConstantTable();
	m_pMeshCom->Render_MagicCircleMesh(m_pShaderCom, m_pDescriptorHeaps,0,2, 3,0,0);
}
void CIceStorm::Render_CrossFilterGameObject(const _float& fTimeDelta)
{
	Set_ConstantTable();
	m_pMeshCom->Render_StaticMesh(m_pCrossFilterShaderCom);
}
HRESULT CIceStorm::Add_Component(wstring wstrMeshTag)
{
	Engine::NULL_CHECK_RETURN(m_pComponentMgr, E_FAIL);

	// Mesh
	m_pMeshCom = static_cast<Engine::CMesh*>(m_pComponentMgr->Clone_Component(wstrMeshTag.c_str(), Engine::COMPONENTID::ID_STATIC));
	Engine::NULL_CHECK_RETURN(m_pMeshCom, E_FAIL);
	m_pMeshCom->AddRef();
	m_mapComponent[Engine::ID_STATIC].emplace(L"Com_Mesh", m_pMeshCom);


	// Shader
	m_pShaderCom = static_cast<Engine::CShaderMeshEffect*>(m_pComponentMgr->Clone_Component(L"ShaderMeshEffect", Engine::COMPONENTID::ID_STATIC));
	Engine::NULL_CHECK_RETURN(m_pShaderCom, E_FAIL);
	Engine::FAILED_CHECK_RETURN(m_pShaderCom->Set_PipelineStatePass(3), E_FAIL);
	m_pShaderCom->AddRef();
	m_mapComponent[Engine::ID_STATIC].emplace(L"Com_Shader", m_pShaderCom);

	m_pCrossFilterShaderCom = static_cast<Engine::CShaderMesh*>(m_pComponentMgr->Clone_Component(L"ShaderMesh", Engine::COMPONENTID::ID_STATIC));
	Engine::NULL_CHECK_RETURN(m_pCrossFilterShaderCom, E_FAIL);
	m_pCrossFilterShaderCom->AddRef();
	Engine::FAILED_CHECK_RETURN(m_pCrossFilterShaderCom->Set_PipelineStatePass(3), E_FAIL);
	m_mapComponent[Engine::ID_STATIC].emplace(L"Com_Shader2", m_pCrossFilterShaderCom);
	return S_OK;
}

void CIceStorm::Set_ConstantTable()
{
	/*__________________________________________________________________________________________________________
	[ Set ConstantBuffer Data ]
	____________________________________________________________________________________________________________*/
	Engine::SHADOW_DESC tShadowDesc = CShadowLightMgr::Get_Instance()->Get_ShadowDesc();

	Engine::CB_SHADER_MESH tCB_ShaderMesh;
	ZeroMemory(&tCB_ShaderMesh, sizeof(Engine::CB_SHADER_MESH));
	tCB_ShaderMesh.matWorld = Engine::CShader::Compute_MatrixTranspose(m_pTransCom->m_matWorld);
	tCB_ShaderMesh.matLightView = Engine::CShader::Compute_MatrixTranspose(tShadowDesc.matLightView);
	tCB_ShaderMesh.matLightProj = Engine::CShader::Compute_MatrixTranspose(tShadowDesc.matLightProj);
	tCB_ShaderMesh.vLightPos = tShadowDesc.vLightPosition;
	tCB_ShaderMesh.fLightPorjFar = tShadowDesc.fLightPorjFar;

	m_fDeltatime += (Engine::CTimerMgr::Get_Instance()->Get_TimeDelta(L"Timer_TimeDelta")) * 0.5f * m_fDeltatimeVelocity;
	
	m_fDeltatime3 += (Engine::CTimerMgr::Get_Instance()->Get_TimeDelta(L"Timer_TimeDelta")) * 0.5f * m_fDeltatimeVelocity2;
	tCB_ShaderMesh.fOffset1 = sin(m_fDeltatime);//����ȿ��
	tCB_ShaderMesh.fOffset2 = m_fDeltatime2;
	tCB_ShaderMesh.fOffset3 = m_fDeltatime3;

	if (m_fLifeTime > 4.f)
	{
		m_fDeltatime2 += (Engine::CTimerMgr::Get_Instance()->Get_TimeDelta(L"Timer_TimeDelta")) * 0.5f;
		tCB_ShaderMesh.fDissolve = m_fDeltatime2;
		if (!m_bisLifeInit)
		{
			m_bisLifeInit = true;
			m_fDeltatimeVelocity2 = 3;
			CEffectMgr::Get_Instance()->Effect_Particle(m_pTransCom->m_vPos, 3,L"Snow");
		
		}
	}
	m_pShaderCom->Get_UploadBuffer_ShaderMesh()->CopyData(0, tCB_ShaderMesh);


	tCB_ShaderMesh.vEmissiveColor.x = 135.f / 255.f;
	tCB_ShaderMesh.vEmissiveColor.y = 150.f / 255.f;
	tCB_ShaderMesh.vEmissiveColor.z = 220.f / 255.f;
	tCB_ShaderMesh.vEmissiveColor.w = 1.f;
	m_pCrossFilterShaderCom->Get_UploadBuffer_ShaderMesh()->CopyData(0, tCB_ShaderMesh);
	if (m_fDeltatime > 1.f)
		m_fDeltatimeVelocity = -1.f;
	else if (m_fDeltatime < 0.f)
		m_fDeltatimeVelocity = 1.f;
	
	if (m_fDeltatime3 > 1.f)
		m_fDeltatimeVelocity2 = -1.f;
	else if (m_fDeltatime3 < 0.f)
	{
		m_fDeltatime3 = 0.0f;
		m_fDeltatimeVelocity2 = 0.f;
	}
}
void CIceStorm::Set_CreateInfo(const _vec3& vScale, const _vec3& vAngle, const _vec3& vPos, const float& fRadius, const float& theta)
{
	m_pTransCom->m_vScale = vScale;
	m_pTransCom->m_vAngle = vAngle;
	m_pTransCom->m_vPos = vPos;
	m_fRadius = fRadius;
	m_fTheta = theta;
	m_pTransCom->m_vAngle.x = static_cast<float>(rand() % 90 - 45);
	m_pTransCom->m_vAngle.y = static_cast<float>(rand() % 360);
	m_pTransCom->m_vAngle.z = static_cast<float>(rand() % 90 - 45);
	m_fLimitScale = 0.15f;
	m_fLifeTime = 0.f;

	m_fDeltatimeVelocity = 0.f;
	m_fDeltatimeVelocity2 = 1.f;
	m_fDeltatime = -1.f;
	m_fDeltatime2 = 0.f;
	m_fDeltatime3 = 0.f;

	m_pTransCom->m_vPos.x += m_fRadius * cos(m_fTheta);
	m_pTransCom->m_vPos.z += m_fRadius * sin(m_fTheta);

	CEffectMgr::Get_Instance()->Effect_Particle(m_pTransCom->m_vPos, 3, L"Snow");

}

Engine::CGameObject* CIceStorm::Create(ID3D12Device * pGraphicDevice, ID3D12GraphicsCommandList * pCommandList,
												wstring wstrMeshTag, 
												const _vec3 & vScale,
												const _vec3 & vAngle,
												const _vec3 & vPos,const float&Radius,const float & fTheta)
{
	CIceStorm* pInstance = new CIceStorm(pGraphicDevice, pCommandList);

	if (FAILED(pInstance->Ready_GameObject(wstrMeshTag, vScale, vAngle, vPos, Radius, fTheta)))
		Engine::Safe_Release(pInstance);

	return pInstance;
}

CIceStorm** CIceStorm::Create_InstancePool(ID3D12Device* pGraphicDevice, ID3D12GraphicsCommandList* pCommandList, const _uint& uiInstanceCnt)
{
	CIceStorm** ppInstance = new (CIceStorm * [uiInstanceCnt]);

	for (_uint i = 0; i < uiInstanceCnt; ++i)
	{
		ppInstance[i] = new CIceStorm(pGraphicDevice, pCommandList);
		ppInstance[i]->m_uiInstanceIdx = i;
		ppInstance[i]->Ready_GameObject(L"IceStorm1", _vec3(0.f), _vec3(0.f), _vec3(0.f),0.f,0.f);
	}

	return ppInstance;
}

void CIceStorm::Free()
{
	Engine::CGameObject::Free();
	Engine::Safe_Release(m_pMeshCom);
	//Engine::Safe_Release(m_pDescriptorHeaps);
	Engine::Safe_Release(m_pShaderCom);
	Engine::Safe_Release(m_pCrossFilterShaderCom);
}
