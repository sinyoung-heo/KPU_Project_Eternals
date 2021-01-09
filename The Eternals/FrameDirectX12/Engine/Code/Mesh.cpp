#include "Mesh.h"
#include "ComponentMgr.h"

USING(Engine)

CMesh::CMesh(ID3D12Device * pGraphicDevice, ID3D12GraphicsCommandList * pCommandList)
	: CComponent(pGraphicDevice, pCommandList)
{
}

CMesh::CMesh(const CMesh & rhs)
	: CComponent(rhs)
	, m_pScene(rhs.m_pScene)
	, m_wstrFilePath(rhs.m_wstrFilePath)
	, m_wstrFullPath(rhs.m_wstrFullPath)
	, m_wstrFileName(rhs.m_wstrFileName)
	, m_wstrVIMeshTag(rhs.m_wstrVIMeshTag)
	, m_wstrAniCtrlTag(rhs.m_wstrAniCtrlTag)
{
	if (m_pScene->mNumMeshes)
		m_pVIMesh	= static_cast<CVIMesh*>(CComponentMgr::Get_Instance()->Clone_Component(m_wstrVIMeshTag, ID_STATIC));

	if (m_pScene->mNumAnimations)
	{
		m_pAniCtrl = static_cast<CAniCtrl*>(CComponentMgr::Get_Instance()->Clone_Component(m_wstrAniCtrlTag, ID_STATIC));
		m_pVIMesh->Set_AniCtrl(m_pAniCtrl);
	}
}

HRESULT CMesh::Ready_Mesh(wstring wstrFilePath, wstring wstrFileName)
{
	/*__________________________________________________________________________________________________________
	wstrFilePath	: ../../Bin/Resource/Mesh/
	wstrFileName	: OOO.X
	____________________________________________________________________________________________________________*/
	m_wstrFilePath = wstrFilePath;
	m_wstrFileName = wstrFileName;

	m_wstrFullPath = wstrFilePath;
	m_wstrFullPath += wstrFileName;

	size_t iLength = wcslen((wchar_t*)m_wstrFullPath.c_str());

	char* pStringPath = new char[2 * iLength + 1];
	wcstombs(pStringPath, (wchar_t*)m_wstrFullPath.c_str(), 2 * iLength + 1);

	string strFullPath = pStringPath;
	Safe_Delete_Array(pStringPath);


	m_pScene = m_Importer.ReadFile(strFullPath.c_str(),
								   aiProcess_MakeLeftHanded |
								   aiPostProcessSteps::aiProcess_FlipWindingOrder |
								   aiProcess_FlipUVs);

	m_wstrVIMeshTag		= L"Prototype" + wstrFileName + L"VIMesh";
	m_wstrAniCtrlTag	= L"Prototype" + wstrFileName + L"AniCtrl";

	CComponent* pComonent = nullptr;
	if (m_pScene->mNumMeshes)
	{
		pComonent = CVIMesh::Create(m_pGraphicDevice, m_pCommandList, m_pScene, m_wstrFilePath);
		NULL_CHECK_RETURN(pComonent, E_FAIL);
		FAILED_CHECK_RETURN(CComponentMgr::Get_Instance()->Add_ComponentPrototype(m_wstrVIMeshTag, ID_STATIC, pComonent), E_FAIL);
	}

	if (m_pScene->mNumAnimations)
	{
		pComonent = CAniCtrl::Create(m_pScene);
		NULL_CHECK_RETURN(pComonent, E_FAIL);
		FAILED_CHECK_RETURN(CComponentMgr::Get_Instance()->Add_ComponentPrototype(m_wstrAniCtrlTag, ID_STATIC, pComonent), E_FAIL);
	}

	return S_OK;
}

void CMesh::Render_DynamicMesh(CShader * pShader)
{
	if (nullptr != m_pVIMesh)
		m_pVIMesh->Render_DynamicMesh(pShader);
}

void CMesh::Render_StaticMesh(CShader * pShader)
{
	if (nullptr != m_pVIMesh)
		m_pVIMesh->Render_StaticMesh(pShader);
}

void CMesh::Render_DynamicMeshShadowDepth(CShader * pShader)
{
	if (nullptr != m_pVIMesh)
		m_pVIMesh->Render_DynamicMeshShadowDepth(pShader);
}

void CMesh::Render_StaticMeshShadowDepth(CShader * pShader)
{
	if (nullptr != m_pVIMesh)
		m_pVIMesh->Render_StaticMeshShadowDepth(pShader);
}

void CMesh::Render_DynamicMesh(ID3D12GraphicsCommandList * pCommandList, 
							   const _int& iContextIdx,
							   CShader * pShader)
{
	if (nullptr != m_pVIMesh && nullptr != pCommandList)
		m_pVIMesh->Render_DynamicMesh(pCommandList, iContextIdx, pShader);
}

void CMesh::Render_StaticMesh(ID3D12GraphicsCommandList * pCommandList, 
							  const _int& iContextIdx,
							  CShader * pShader)
{
	if (nullptr != m_pVIMesh && nullptr != pCommandList)
		m_pVIMesh->Render_StaticMesh(pCommandList, iContextIdx, pShader);
}

void CMesh::Render_DynamicMeshShadowDepth(ID3D12GraphicsCommandList * pCommandList,
										  const _int& iContextIdx,
										  CShader * pShader)
{
	if (nullptr != m_pVIMesh && nullptr != pCommandList)
		m_pVIMesh->Render_DynamicMeshShadowDepth(pCommandList, iContextIdx, pShader);
}

void CMesh::Render_StaticMeshShadowDepth(ID3D12GraphicsCommandList * pCommandList,
										 const _int& iContextIdx,
										 CShader * pShader)
{
	if (nullptr != m_pVIMesh && nullptr != pCommandList)
		m_pVIMesh->Render_StaticMeshShadowDepth(pCommandList, iContextIdx, pShader);
}

void CMesh::Set_AnimationKey(const _uint & uiAniKey)
{
	if (nullptr != m_pAniCtrl)
		m_pAniCtrl->Set_AnimationKey(uiAniKey);
}

void CMesh::Play_Animation(_float fAnimationTime)
{
	if (nullptr != m_pAniCtrl)
		m_pAniCtrl->Play_Animation(fAnimationTime);
}

SKINNING_MATRIX * CMesh::Find_SkinningMatrix(string strBoneName)
{
	if (nullptr != m_pAniCtrl)
		return m_pAniCtrl->Find_SkinningMatrix(strBoneName);

	return nullptr;
}

CComponent * CMesh::Clone()
{
	return new CMesh(*this);
}

CMesh * CMesh::Create(ID3D12Device * pGraphicDevice, 
					  ID3D12GraphicsCommandList * pCommandList,
					  wstring wstrFilePath, 
					  wstring wstrFileName)
{
	CMesh* pInstance = new CMesh(pGraphicDevice, pCommandList);

	if (FAILED(pInstance->Ready_Mesh(wstrFilePath, wstrFileName)))
		Engine::Safe_Release(pInstance);

	return pInstance;
}

void CMesh::Free()
{
	Safe_Release(m_pVIMesh);

	if (m_pAniCtrl != nullptr)
		Safe_Release(m_pAniCtrl);

	if (!m_bIsClone)
	{
		m_pScene = nullptr;
	}
}