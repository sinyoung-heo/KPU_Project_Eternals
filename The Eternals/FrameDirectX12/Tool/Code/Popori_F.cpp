#include "stdafx.h"
#include "Popori_F.h"

#include "GraphicDevice.h"
#include "DirectInput.h"
#include "ObjectMgr.h"
#include "LightMgr.h"
#include "Font.h"
#include "RenderTarget.h"

CPopori_F::CPopori_F(ID3D12Device* pGraphicDevice, ID3D12GraphicsCommandList* pCommandList)
	: Engine::CGameObject(pGraphicDevice, pCommandList)
{
}

CPopori_F::CPopori_F(const CPopori_F& rhs)
	: Engine::CGameObject(rhs)
	, m_wstrMeshTag(rhs.m_wstrMeshTag)
{
}

HRESULT CPopori_F::Ready_GameObject(wstring wstrMeshTag,
	const _vec3& vScale,
	const _vec3& vAngle,
	const _vec3& vPos)
{
	Engine::FAILED_CHECK_RETURN(Engine::CGameObject::Ready_GameObject(true, true, true), E_FAIL);
	Engine::FAILED_CHECK_RETURN(Add_Component(wstrMeshTag), E_FAIL);
	m_pTransCom->m_vScale = vScale;
	m_pTransCom->m_vAngle = vAngle;
	m_pTransCom->m_vPos = vPos;
	Engine::CGameObject::SetUp_BoundingBox(&(m_pTransCom->m_matWorld),
										   m_pTransCom->m_vScale,
										   m_pMeshCom->Get_CenterPos(),
										   m_pMeshCom->Get_MinVector(),
										   m_pMeshCom->Get_MaxVector());
	m_pInfoCom->m_fSpeed = 15.0f;

	/*__________________________________________________________________________________________________________
	[ 애니메이션 설정 ]
	____________________________________________________________________________________________________________*/
	m_uiAnimIdx = 1;
	m_pMeshCom->Set_AnimationKey(m_uiAnimIdx);

	/*__________________________________________________________________________________________________________
	[ Collider Bone Setting ]
	____________________________________________________________________________________________________________*/
	Engine::SKINNING_MATRIX* pmatSkinning = nullptr;
	_matrix* pmatParent = nullptr;

	// ColliderSphere
	// pmatSkinning	= m_pMeshCom->Find_SkinningMatrix("Bip01-L-Hand");
	pmatSkinning = m_pMeshCom->Find_SkinningMatrix("Bip01-R-Hand");
	pmatParent = &(m_pTransCom->m_matWorld);
	Engine::NULL_CHECK_RETURN(pmatSkinning, E_FAIL);
	m_pColliderSphereCom->Set_SkinningMatrix(pmatSkinning);		// Bone Matrix
	m_pColliderSphereCom->Set_ParentMatrix(pmatParent);			// Parent Matrix
	m_pColliderSphereCom->Set_Scale(_vec3(3.f, 3.f, 3.f));		// Collider Scale
	m_pColliderSphereCom->Set_Radius(m_pTransCom->m_vScale);	// Collider Radius

	// ColliderBox
	pmatSkinning = m_pMeshCom->Find_SkinningMatrix("Bip01-R-Hand");
	pmatParent = &(m_pTransCom->m_matWorld);
	Engine::NULL_CHECK_RETURN(pmatSkinning, E_FAIL);
	m_pColliderBoxCom->Set_SkinningMatrix(pmatSkinning);	// Bone Matrix
	m_pColliderBoxCom->Set_ParentMatrix(pmatParent);		// Parent Matrix
	m_pColliderBoxCom->Set_Scale(_vec3(3.f, 3.f, 3.f));		// Collider Scale
	m_pColliderBoxCom->Set_Extents(m_pTransCom->m_vScale);	// Box Offset From Center

	return S_OK;
}

HRESULT CPopori_F::LateInit_GameObject()
{
	// SetUp Shader ConstantBuffer
	m_pShaderCom->SetUp_ShaderConstantBuffer((_uint)(m_pMeshCom->Get_DiffTexture().size()));
	m_pShadowCom->SetUp_ShaderConstantBuffer((_uint)(m_pMeshCom->Get_DiffTexture().size()));

	return S_OK;
}

_int CPopori_F::Update_GameObject(const _float& fTimeDelta)
{
	Engine::FAILED_CHECK_RETURN(Engine::CGameObject::LateInit_GameObject(), E_FAIL);

	if (m_bIsDead)
		return DEAD_OBJ;

	/*__________________________________________________________________________________________________________
	[ TransCom - Update WorldMatrix ]
	____________________________________________________________________________________________________________*/
	Engine::CGameObject::Update_GameObject(fTimeDelta);


	return NO_EVENT;
}

_int CPopori_F::LateUpdate_GameObject(const _float& fTimeDelta)
{
	Engine::NULL_CHECK_RETURN(m_pRenderer, -1);

	/*__________________________________________________________________________________________________________
	[ Renderer - Add Render Group ]
	____________________________________________________________________________________________________________*/
	Engine::FAILED_CHECK_RETURN(m_pRenderer->Add_Renderer(Engine::CRenderer::RENDER_NONALPHA, this), -1);

	/*__________________________________________________________________________________________________________
	[ Animation KeyFrame Index ]
	____________________________________________________________________________________________________________*/
	m_ui3DMax_NumFrame = *(m_pMeshCom->Get_3DMaxNumFrame());
	m_ui3DMax_CurFrame = *(m_pMeshCom->Get_3DMaxCurFrame());

	return NO_EVENT;
}

void CPopori_F::Render_GameObject(const _float& fTimeDelta)
{
	/*__________________________________________________________________________________________________________
	[ Play Animation ]
	____________________________________________________________________________________________________________*/
	m_pMeshCom->Set_AnimationKey(m_uiAnimIdx);
	m_pMeshCom->Play_Animation(fTimeDelta * TPS);

	Set_ConstantTable();
	m_pMeshCom->Render_DynamicMesh(m_pShaderCom);
}

void CPopori_F::Render_ShadowDepth(const _float& fTimeDelta)
{
	Set_ConstantTableShadowDepth();
	m_pMeshCom->Render_DynamicMeshShadowDepth(m_pShadowCom);
}

void CPopori_F::Render_GameObject(const _float& fTimeDelta,
								  ID3D12GraphicsCommandList* pCommandList,
								  const _int& iContextIdx)
{
	/*__________________________________________________________________________________________________________
	[ Play Animation ]
	____________________________________________________________________________________________________________*/
	m_pMeshCom->Set_AnimationKey(m_uiAnimIdx);
	m_pMeshCom->Play_Animation(fTimeDelta * TPS);

	Set_ConstantTable();
	m_pMeshCom->Render_DynamicMesh(pCommandList, iContextIdx, m_pShaderCom);
}

void CPopori_F::Render_ShadowDepth(const _float& fTimeDelta,
	ID3D12GraphicsCommandList* pCommandList,
	const _int& iContextIdx)
{
	Set_ConstantTableShadowDepth();
	m_pMeshCom->Render_DynamicMeshShadowDepth(pCommandList, iContextIdx, m_pShadowCom);
}

HRESULT CPopori_F::Add_Component(wstring wstrMeshTag)
{
	Engine::NULL_CHECK_RETURN(m_pComponentMgr, E_FAIL);

	// Mesh
	m_pMeshCom = static_cast<Engine::CMesh*>(m_pComponentMgr->Clone_Component(wstrMeshTag.c_str(), Engine::COMPONENTID::ID_STATIC));
	Engine::NULL_CHECK_RETURN(m_pMeshCom, E_FAIL);
	m_pMeshCom->AddRef();
	m_mapComponent[Engine::ID_STATIC].emplace(L"Com_Mesh", m_pMeshCom);

	// Shader
	m_pShaderCom = static_cast<Engine::CShaderMesh*>(m_pComponentMgr->Clone_Component(L"ShaderMesh", Engine::COMPONENTID::ID_STATIC));
	Engine::NULL_CHECK_RETURN(m_pShaderCom, E_FAIL);
	m_pShaderCom->AddRef();
	Engine::FAILED_CHECK_RETURN(m_pShaderCom->Set_PipelineStatePass(0), E_FAIL);
	m_mapComponent[Engine::ID_STATIC].emplace(L"Com_Shader", m_pShaderCom);

	// Shadow
	m_pShadowCom = static_cast<Engine::CShaderShadow*>(m_pComponentMgr->Clone_Component(L"ShaderShadow", Engine::COMPONENTID::ID_STATIC));
	Engine::NULL_CHECK_RETURN(m_pShadowCom, E_FAIL);
	m_pShadowCom->AddRef();
	Engine::FAILED_CHECK_RETURN(m_pShadowCom->Set_PipelineStatePass(0), E_FAIL);
	m_mapComponent[Engine::ID_STATIC].emplace(L"Com_Shadow", m_pShadowCom);

	// Collider - Sphere
	m_pColliderSphereCom = static_cast<Engine::CColliderSphere*>(m_pComponentMgr->Clone_Component(L"ColliderSphere", Engine::COMPONENTID::ID_DYNAMIC));
	Engine::NULL_CHECK_RETURN(m_pColliderSphereCom, E_FAIL);
	m_pColliderSphereCom->AddRef();
	m_mapComponent[Engine::ID_DYNAMIC].emplace(L"Com_ColliderSphere", m_pColliderSphereCom);

	// Collider - Box
	m_pColliderBoxCom = static_cast<Engine::CColliderBox*>(m_pComponentMgr->Clone_Component(L"ColliderBox", Engine::COMPONENTID::ID_DYNAMIC));
	Engine::NULL_CHECK_RETURN(m_pColliderBoxCom, E_FAIL);
	m_pColliderBoxCom->AddRef();
	m_mapComponent[Engine::ID_DYNAMIC].emplace(L"Com_ColliderBox", m_pColliderBoxCom);

	return S_OK;
}

void CPopori_F::Set_ConstantTable()
{
	_matrix* pmatView = Engine::CGraphicDevice::Get_Instance()->Get_Transform(Engine::VIEW);
	_matrix* pmatProj = Engine::CGraphicDevice::Get_Instance()->Get_Transform(Engine::PROJECTION);

	if (nullptr == pmatView || nullptr == pmatProj)
		return;

	/*__________________________________________________________________________________________________________
	[ Matrix Info ]
	____________________________________________________________________________________________________________*/
	Engine::CB_MATRIX_DESC	tCB_MatrixDesc;
	ZeroMemory(&tCB_MatrixDesc, sizeof(Engine::CB_MATRIX_DESC));
	XMStoreFloat4x4(&tCB_MatrixDesc.matWVP, XMMatrixTranspose(m_pTransCom->m_matWorld * (*pmatView) * (*pmatProj)));
	XMStoreFloat4x4(&tCB_MatrixDesc.matWorld, XMMatrixTranspose(m_pTransCom->m_matWorld));
	XMStoreFloat4x4(&tCB_MatrixDesc.matView, XMMatrixTranspose(*pmatView));
	XMStoreFloat4x4(&tCB_MatrixDesc.matProj, XMMatrixTranspose(*pmatProj));

	m_pShaderCom->Get_UploadBuffer_MatrixDesc()->CopyData(0, tCB_MatrixDesc);
}

void CPopori_F::Set_ConstantTableShadowDepth()
{
	m_tShadowInfo = CShadowLightMgr::Get_Instance()->m_tShadowDesc;

	/*__________________________________________________________________________________________________________
	[ ShadowDepth ]
	____________________________________________________________________________________________________________*/
	Engine::CB_SHADOWDEPTH_DESC	tCB_ShadowDepthDesc;
	ZeroMemory(&tCB_ShadowDepthDesc, sizeof(Engine::CB_SHADOWDEPTH_DESC));
	XMStoreFloat4x4(&tCB_ShadowDepthDesc.matWVP, XMMatrixTranspose(m_pTransCom->m_matWorld * m_tShadowInfo.matLightView * m_tShadowInfo.matLightProj));
	XMStoreFloat4x4(&tCB_ShadowDepthDesc.matWorld, XMMatrixTranspose(m_pTransCom->m_matWorld));
	XMStoreFloat4x4(&tCB_ShadowDepthDesc.matLightView, XMMatrixTranspose(m_tShadowInfo.matLightView));
	XMStoreFloat4x4(&tCB_ShadowDepthDesc.matLightProj, XMMatrixTranspose(m_tShadowInfo.matLightProj));
	tCB_ShadowDepthDesc.vLightPosition = m_tShadowInfo.vLightPosition;
	tCB_ShadowDepthDesc.fLightPorjFar = m_tShadowInfo.fLightPorjFar;

	m_pShadowCom->Get_UploadBuffer_ShadowDepthDesc()->CopyData(0, tCB_ShadowDepthDesc);

	/*__________________________________________________________________________________________________________
	[ ShadowInfo ]
	____________________________________________________________________________________________________________*/
	Engine::CB_SHADOW_DESC	tCB_ShadowDesc;
	ZeroMemory(&tCB_ShadowDesc, sizeof(Engine::CB_SHADOW_DESC));
	XMStoreFloat4x4(&tCB_ShadowDesc.matLightView, XMMatrixTranspose(m_tShadowInfo.matLightView));
	XMStoreFloat4x4(&tCB_ShadowDesc.matLightProj, XMMatrixTranspose(m_tShadowInfo.matLightProj));
	tCB_ShadowDesc.vLightPosition = m_tShadowInfo.vLightPosition;
	tCB_ShadowDesc.fLightPorjFar = m_tShadowInfo.fLightPorjFar;

	m_pShaderCom->Get_UploadBuffer_ShadowDesc()->CopyData(0, tCB_ShadowDesc);

}


CPopori_F* CPopori_F::Create(ID3D12Device* pGraphicDevice,
							 ID3D12GraphicsCommandList* pCommandList,
							 wstring wstrMeshTag,
							 const _vec3& vScale,
							 const _vec3& vAngle,
							 const _vec3& vPos)
{
	CPopori_F* pInstance = new CPopori_F(pGraphicDevice, pCommandList);

	if (FAILED(pInstance->Ready_GameObject(wstrMeshTag, vScale, vAngle, vPos)))
		Engine::Safe_Release(pInstance);

	return pInstance;
}

void CPopori_F::Free()
{
	Engine::CGameObject::Free();

	Engine::Safe_Release(m_pMeshCom);
	Engine::Safe_Release(m_pShaderCom);
	Engine::Safe_Release(m_pShadowCom);
	Engine::Safe_Release(m_pColliderSphereCom);
	Engine::Safe_Release(m_pColliderBoxCom);
}
