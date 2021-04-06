#include "stdafx.h"
#include "EffectTrail.h"
#include "Renderer.h"
#include "ObjectMgr.h"
#include "GraphicDevice.h"

CEffectTrail::CEffectTrail(ID3D12Device* pGraphicDevice, ID3D12GraphicsCommandList* pCommandList)
	: Engine::CGameObject(pGraphicDevice, pCommandList)
{
}

HRESULT CEffectTrail::Ready_GameObject(wstring wstrTextureTag, 
									   const _uint& uiTexIdx, 
									   const _vec3& vScale, 
									   const _vec3& vAngle)
{
	Engine::FAILED_CHECK_RETURN(Engine::CGameObject::Ready_GameObject(), E_FAIL);
	Engine::FAILED_CHECK_RETURN(Add_Component(wstrTextureTag), E_FAIL);

	m_wstrTextureTag      = wstrTextureTag;
	m_uiTexIdx            = uiTexIdx;
	m_pTransCom->m_vScale = vScale;
	m_pTransCom->m_vAngle = vAngle;
	m_pTransCom->m_vPos	  = _vec3(0.0f);

	// Trail Buffer
	for (_uint i = 0; i < m_arrMax.size(); ++i)
		m_arrMax[i] = _vec3(0.0f);

	Engine::FAILED_CHECK_RETURN(m_pShaderCom->Set_PipelineStatePass(7), E_FAIL);

	return S_OK;
}

HRESULT CEffectTrail::LateInit_GameObject()
{
	// SetUp Shader ConstantBuffer
	m_pShaderCom->SetUp_ShaderConstantBuffer();

	return S_OK;
}

_int CEffectTrail::Update_GameObject(const _float& fTimeDelta)
{
	Engine::FAILED_CHECK_RETURN(Engine::CGameObject::LateInit_GameObject(), E_FAIL);

	if (m_bIsDead)
		return DEAD_OBJ;

	/*__________________________________________________________________________________________________________
	[ Renderer - Add Render Group ]
	____________________________________________________________________________________________________________*/
	Engine::FAILED_CHECK_RETURN(m_pRenderer->Add_Renderer(Engine::CRenderer::RENDER_ALPHA, this), -1);

	/*__________________________________________________________________________________________________________
	[ TransCom - Update WorldMatrix ]
	____________________________________________________________________________________________________________*/
	Engine::CGameObject::Update_GameObject(fTimeDelta);

	return NO_EVENT;
}

_int CEffectTrail::LateUpdate_GameObject(const _float& fTimeDelta)
{
	return NO_EVENT;
}

void CEffectTrail::Render_GameObject(const _float& fTimeDelta)
{
	Set_ConstantTable();
	// Vertex���� ���� CopyData.
	//memcpy(m_pVetexData, m_arrVertices.data(), m_uiVB_ByteSize);

	m_pBufferCom->Begin_Buffer();
	m_pShaderCom->Begin_Shader(m_pTextureCom->Get_TexDescriptorHeap(), 0, m_uiTexIdx, Engine::MATRIXID::PROJECTION);
	m_pBufferCom->Render_Buffer();

	//m_pCommandList->IASetVertexBuffers(0, 1, &Get_VertexBufferView());
	//m_pCommandList->IASetIndexBuffer(&Get_IndexBufferView());
	//m_pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//m_pCommandList->DrawIndexedInstanced(m_tSubMeshGeometry.uiIndexCount, 1, 0, 0, 0);	
}

void CEffectTrail::SetUp_TrailByCatmullRom(_vec3* vMin, _vec3* vMax)
{
	for (_int i = TRAIL_SIZE - 2; i > 0; --i)
		m_arrMax[i] = m_arrMax[i - 1];

	m_arrMax[0] = *vMax;

	// CatmullRom
	for (_int i = 2; i < TRAIL_SIZE - 2; ++i)
	{
		_vec3 catmullrom;
		catmullrom.Vector3CatmullRom(m_arrMax[i - 2], m_arrMax[i - 1], m_arrMax[i], m_arrMax[i + 1], 0.5f);

		m_arrMax[i + 1] = (m_arrMax[i] + m_arrMax[i + 1]) * 0.5f;
		m_arrMax[i]     = (catmullrom + m_arrMax[i]) * 0.5f;
		m_arrMax[i - 1] = (catmullrom + m_arrMax[i - 1]) * 0.5f;
	}

	// SetUp Vertex
	m_pBufferCom->Get_ArrayVerteices()[0].vPos = _vec3(vMin->x, vMin->y, vMin->z);
	m_pBufferCom->Get_ArrayVerteices()[0].vTexUV = _vec2(0.0f, 1.0f);

	for (int i = 0; i < TRAIL_SIZE - 1; ++i)
	{
		float fTex = _float(i) / (TRAIL_SIZE - 2);

		if (i > 2 && i < TRAIL_SIZE - 2)
		{
			m_pBufferCom->Get_ArrayVerteices()[i + 1].vPos = _vec3(m_arrMax[i].x, m_arrMax[i].y, m_arrMax[i].z);
			m_pBufferCom->Get_ArrayVerteices()[i + 1].vTexUV = _vec2(fTex, 0.f);
		}
		else
		{
			m_pBufferCom->Get_ArrayVerteices()[i + 1].vPos = _vec3(m_arrMax[i].x, m_arrMax[i].y, m_arrMax[i].z);
			m_pBufferCom->Get_ArrayVerteices()[i + 1].vTexUV = _vec2(fTex, 0.f);
		}
	}
}

HRESULT CEffectTrail::Add_Component(wstring wstrTextureTag)
{
	Engine::NULL_CHECK_RETURN(m_pComponentMgr, E_FAIL);

	// TrailTex
	m_pBufferCom = static_cast<Engine::CTrailTex*>(m_pComponentMgr->Clone_Component(L"TrailTex", Engine::COMPONENTID::ID_STATIC));
	Engine::NULL_CHECK_RETURN(m_pBufferCom, E_FAIL);
	m_pBufferCom->AddRef();
	m_mapComponent[Engine::ID_STATIC].emplace(L"Com_Buffer", m_pBufferCom);

	// Texture
	m_pTextureCom = static_cast<Engine::CTexture*>(m_pComponentMgr->Clone_Component(wstrTextureTag, Engine::COMPONENTID::ID_STATIC));
	Engine::NULL_CHECK_RETURN(m_pTextureCom, E_FAIL);
	m_pTextureCom->AddRef();
	m_mapComponent[Engine::ID_STATIC].emplace(L"Com_Texture", m_pTextureCom);

	// Shader
	m_pShaderCom = static_cast<Engine::CShaderTexture*>(m_pComponentMgr->Clone_Component(L"ShaderTexture", Engine::COMPONENTID::ID_STATIC));
	Engine::NULL_CHECK_RETURN(m_pShaderCom, E_FAIL);
	m_pShaderCom->AddRef();;
	m_mapComponent[Engine::ID_STATIC].emplace(L"Com_Shader", m_pShaderCom);

	return S_OK;
}

void CEffectTrail::Set_ConstantTable()
{
	/*__________________________________________________________________________________________________________
	[ Set ConstantBuffer Data ]
	____________________________________________________________________________________________________________*/
	Engine::CB_SHADER_TEXTURE tCB_ShaderTexture;
	ZeroMemory(&tCB_ShaderTexture, sizeof(Engine::CB_SHADER_TEXTURE));
	tCB_ShaderTexture.matWorld	= Engine::CShader::Compute_MatrixTranspose(INIT_MATRIX);

	m_pShaderCom->Get_UploadBuffer_ShaderTexture()->CopyData(0, tCB_ShaderTexture);
}

CEffectTrail* CEffectTrail::Create(ID3D12Device* pGraphicDevice, ID3D12GraphicsCommandList* pCommandList,
								   wstring wstrTextureTag,
								   const _uint& uiTexIdx, 
								   const _vec3& vScale, 
								   const _vec3& vAngle)
{
	CEffectTrail* pInstance = new CEffectTrail(pGraphicDevice, pCommandList);

	if (FAILED(pInstance->Ready_GameObject(wstrTextureTag, uiTexIdx, vScale, vAngle)))
		Engine::Safe_Release(pInstance);

	return pInstance;
}

void CEffectTrail::Free()
{
	Engine::CGameObject::Free();
	Engine::Safe_Release(m_pBufferCom);
	Engine::Safe_Release(m_pShaderCom);
	Engine::Safe_Release(m_pTextureCom);

	//Engine::Safe_Release(m_pVB_CPU);
	//Engine::Safe_Release(m_pIB_CPU);
	//Engine::Safe_Release(m_pVB_GPU);
	//Engine::Safe_Release(m_pIB_GPU);
}