#include "stdafx.h"
#include "Vergos.h"
#include "InstancePoolMgr.h"
#include "GraphicDevice.h"
#include "DirectInput.h"
#include "ObjectMgr.h"
#include "LightMgr.h"
#include "Font.h"
#include "RenderTarget.h"
#include "TimeMgr.h"
#include "CollisionTick.h"
#include "InstancePoolMgr.h"
#include "NormalMonsterHpGauge.h"
#include "BreathEffect.h"
#include "GameUIRoot.h"
#include "VergosHpGauge.h"

CVergos::CVergos(ID3D12Device* pGraphicDevice, ID3D12GraphicsCommandList* pCommandList)
	: Engine::CGameObject(pGraphicDevice, pCommandList)
	, m_pPacketMgr(CPacketMgr::Get_Instance())
	, m_pServerMath(CServerMath::Get_Instance())
	, m_pInstancePoolMgr(CInstancePoolMgr::Get_Instance())
{
}

HRESULT CVergos::Ready_GameObject(wstring wstrMeshTag, wstring wstrNaviMeshTag, const _vec3& vScale, const _vec3& vAngle, const _vec3& vPos)
{
	Engine::FAILED_CHECK_RETURN(Engine::CGameObject::Ready_GameObject(true, true, true, true), E_FAIL);
	Engine::FAILED_CHECK_RETURN(Add_Component(wstrMeshTag, wstrNaviMeshTag), E_FAIL);
	m_pTransCom->m_vScale = vScale;
	m_pTransCom->m_vAngle = vAngle;
	m_pTransCom->m_vPos = vPos;
	m_pNaviMeshCom->Set_CurrentCellIndex(m_pNaviMeshCom->Get_CurrentPositionCellIndex(vPos));

	Engine::CGameObject::SetUp_BoundingBox(&(m_pTransCom->m_matWorld),
										   m_pTransCom->m_vScale,
										   m_pMeshCom->Get_CenterPos(),
										   m_pMeshCom->Get_MinVector(),
										   m_pMeshCom->Get_MaxVector());
	Engine::CGameObject::SetUp_BoundingSphere(&(m_pTransCom->m_matWorld),
											  m_pTransCom->m_vScale,
											  _vec3(128.0f),
											  _vec3(2.0f, 37.f, 0.0f));
	m_wstrCollisionTag = L"Monster_SingleCollider";
	m_lstCollider.push_back(m_pBoundingSphereCom);

	m_pInfoCom->m_fSpeed = 1.f;
	m_bIsMoveStop = true;

	/*__________________________________________________________________________________________________________
	[ 局聪皋捞记 汲沥 ]
	____________________________________________________________________________________________________________*/
	m_uiAnimIdx = 0;
	m_iMonsterStatus = Vergos::A_WAIT;

	// Create HpGauge
	Engine::FAILED_CHECK_RETURN(Create_HpGauge(), E_FAIL);

	return S_OK;
}

HRESULT CVergos::LateInit_GameObject()
{
	// SetUp Shader ConstantBuffer
	m_pShaderCom->SetUp_ShaderConstantBuffer((_uint)(m_pMeshCom->Get_DiffTexture().size()));
	m_pShadowCom->SetUp_ShaderConstantBuffer((_uint)(m_pMeshCom->Get_DiffTexture().size()));

	// MiniMap
	Engine::FAILED_CHECK_RETURN(Engine::CGameObject::SetUp_MiniMapComponent(3), E_FAIL);

	BazierPos[0] = _vec3(403.f, 0.f, 360.f);
	BazierPos[1] = _vec3(390.f, 0.f, 347.5f);
	BazierPos[2] = _vec3(350.f, 0.f, 362.f);

	return S_OK;
}

_int CVergos::Update_GameObject(const _float& fTimeDelta)
{
	Engine::FAILED_CHECK_RETURN(Engine::CGameObject::LateInit_GameObject(), E_FAIL);

	EffectLoop(fTimeDelta);
	
	if (m_bIsDead)
		return DEAD_OBJ;

	if (m_bIsReturn)
	{
		if (nullptr != m_pHpGaugeRoot)
		{
			m_pHpGaugeRoot->Set_IsActive(false);
			m_pHpGaugeRoot->Set_IsChildActive(false);
		}

		m_iSNum = -1;
		m_bIsStartDissolve = false;
		m_fDissolve = -0.05f;
		m_bIsResetNaviMesh = false;
		Return_Instance(CInstancePoolMgr::Get_Instance()->Get_MonsterVergosPool(), m_uiInstanceIdx);

		return RETURN_OBJ;
	}

	if (m_iSNum == -1)
	{
		m_bIsStartDissolve = false;
		m_bIsResetNaviMesh = false;
		m_fDissolve = -0.05f;
		Return_Instance(CInstancePoolMgr::Get_Instance()->Get_MonsterVergosPool(), m_uiInstanceIdx);

		return RETURN_OBJ;
	}

	if (!m_bIsResetNaviMesh)
	{
		m_bIsResetNaviMesh = true;
		m_pNaviMeshCom->Set_CurrentCellIndex(m_pNaviMeshCom->Get_CurrentPositionCellIndex(m_pTransCom->m_vPos));
	}

	// Create CollisionTick
	if (m_pMeshCom->Is_BlendingComplete())
		SetUp_CollisionTick(fTimeDelta);

	SetUp_Dissolve(fTimeDelta);

	// Angle Linear Interpolation
	SetUp_AngleInterpolation(fTimeDelta);
	
	/* Animation AI */
	Change_Animation(fTimeDelta);

	/*__________________________________________________________________________________________________________
	[ Play Animation ]
	____________________________________________________________________________________________________________*/
	if (!m_bIsStartDissolve)
	{
		m_pMeshCom->Set_AnimationKey(m_uiAnimIdx);
		m_pMeshCom->Play_Animation(fTimeDelta * TPS);
		m_ui3DMax_NumFrame = *(m_pMeshCom->Get_3DMaxNumFrame());
		m_ui3DMax_CurFrame = *(m_pMeshCom->Get_3DMaxCurFrame());
	}

	/*__________________________________________________________________________________________________________
	[ Renderer - Add Render Group ]
	____________________________________________________________________________________________________________*/
	if (!g_bIsStartSkillCameraEffect && !g_bIsCinemaStart)
		Engine::FAILED_CHECK_RETURN(m_pRenderer->Add_Renderer(Engine::CRenderer::RENDER_NONALPHA, this), -1);
	Engine::FAILED_CHECK_RETURN(m_pRenderer->Add_Renderer(Engine::CRenderer::RENDER_MINIMAP, this), -1);

	/*__________________________________________________________________________________________________________
	[ Collision - Add Collision List ]
	____________________________________________________________________________________________________________*/
	if (!m_bIsStartDissolve && Vergos::A_DEATH != m_iMonsterStatus)
		m_pCollisonMgr->Add_CollisionCheckList(this);

	/*__________________________________________________________________________________________________________
	[ TransCom - Update WorldMatrix ]
	____________________________________________________________________________________________________________*/
	Engine::CGameObject::Update_GameObject(fTimeDelta);
	Engine::CGameObject::SetUp_MiniMapRandomY();

	return NO_EVENT;
}

_int CVergos::LateUpdate_GameObject(const _float& fTimeDelta)
{
	Engine::NULL_CHECK_RETURN(m_pRenderer, -1);

	SetUp_HpGauge(fTimeDelta);

	Set_ConstantTableShadowDepth();
	Set_ConstantTable();

	return NO_EVENT;
}

void CVergos::Send_PacketToServer()
{
}

void CVergos::Render_MiniMap(const _float& fTimeDelta)
{
	Set_ConstantTableMiniMap();

	m_pShaderMiniMap->Begin_Shader(m_pTextureMiniMap->Get_TexDescriptorHeap(), 
								   0, 
								   m_uiMiniMapTexIdx, 
								   Engine::MATRIXID::TOP_VIEW);
	m_pBufferMiniMap->Begin_Buffer();
	m_pBufferMiniMap->Render_Buffer();
}

void CVergos::Render_GameObject(const _float& fTimeDelta, ID3D12GraphicsCommandList* pCommandList, const _int& iContextIdx)
{
	Render_HitEffect(fTimeDelta);
	m_pMeshCom->Render_DynamicMesh(pCommandList, iContextIdx, m_pShaderCom);
}

void CVergos::Render_ShadowDepth(const _float& fTimeDelta, ID3D12GraphicsCommandList* pCommandList, const _int& iContextIdx)
{
	if (!m_bIsStartDissolve)
	{
		m_pMeshCom->Render_DynamicMeshShadowDepth(pCommandList, iContextIdx, m_pShadowCom);
	}
}

HRESULT CVergos::Add_Component(wstring wstrMeshTag, wstring wstrNaviMeshTag)
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
	Engine::FAILED_CHECK_RETURN(m_pShaderCom->Set_PipelineStatePass(7), E_FAIL);
	m_mapComponent[Engine::ID_STATIC].emplace(L"Com_Shader", m_pShaderCom);

	// Shadow
	m_pShadowCom = static_cast<Engine::CShaderShadow*>(m_pComponentMgr->Clone_Component(L"ShaderShadow", Engine::COMPONENTID::ID_STATIC));
	Engine::NULL_CHECK_RETURN(m_pShadowCom, E_FAIL);
	m_pShadowCom->AddRef();
	Engine::FAILED_CHECK_RETURN(m_pShadowCom->Set_PipelineStatePass(0), E_FAIL);
	m_mapComponent[Engine::ID_STATIC].emplace(L"Com_Shadow", m_pShadowCom);

	// NaviMesh
	m_pNaviMeshCom = static_cast<Engine::CNaviMesh*>(m_pComponentMgr->Clone_Component(wstrNaviMeshTag.c_str(), Engine::ID_DYNAMIC));
	Engine::NULL_CHECK_RETURN(m_pNaviMeshCom, E_FAIL);
	m_pNaviMeshCom->AddRef();
	m_mapComponent[Engine::ID_STATIC].emplace(L"Com_NaviMesh", m_pNaviMeshCom);

	return S_OK;
}

HRESULT CVergos::Create_HpGauge()
{
	{
		wifstream fin { L"../../Bin/ToolData/2DUIBossHpGaugeV2.2DUI" };
		if (fin.fail())
			return E_FAIL;

		// RootUI Data
		wstring wstrDataFilePath   = L"";			// DataFilePath
		wstring wstrRootObjectTag  = L"";			// ObjectTag
		_vec3	vPos               = _vec3(0.0f);	// Pos
		_vec3	vScale             = _vec3(1.0f);	// Scale
		_long	UIDepth            = 0;				// UIDepth
		_bool	bIsSpriteAnimation = false;			// IsSpriteAnimation
		_float	fFrameSpeed        = 0.0f;			// FrameSpeed
		_vec3	vRectPosOffset     = _vec3(0.0f);	// RectPosOffset
		_vec3	vRectScale         = _vec3(1.0f);	// RectScale
		_int	iChildUISize       = 0;				// ChildUI Size

		// ChildUI Data
		vector<wstring> vecDataFilePath;
		vector<wstring> vecObjectTag;
		vector<_vec3>	vecPos;
		vector<_vec3>	vecScale;
		vector<_long>	vecUIDepth;
		vector<_int>	vecIsSpriteAnimation;
		vector<_float>	vecFrameSpeed;
		vector<_vec3>	vecRectPosOffset;
		vector<_vec3>	vecRectScale;

		while (true)
		{
			fin >> wstrDataFilePath
				>> wstrRootObjectTag
				>> vPos.x
				>> vPos.y
				>> vScale.x
				>> vScale.y
				>> UIDepth
				>> bIsSpriteAnimation
				>> fFrameSpeed
				>> vRectPosOffset.x
				>> vRectPosOffset.y
				>> vRectScale.x
				>> vRectScale.y
				>> iChildUISize;

			vecDataFilePath.resize(iChildUISize);
			vecObjectTag.resize(iChildUISize);
			vecPos.resize(iChildUISize);
			vecScale.resize(iChildUISize);
			vecUIDepth.resize(iChildUISize);
			vecIsSpriteAnimation.resize(iChildUISize);
			vecFrameSpeed.resize(iChildUISize);
			vecRectPosOffset.resize(iChildUISize);
			vecRectScale.resize(iChildUISize);

			for (_int i = 0; i < iChildUISize; ++i)
			{
				fin >> vecDataFilePath[i]			// DataFilePath
					>> vecObjectTag[i]				// Object Tag
					>> vecPos[i].x					// Pos X
					>> vecPos[i].y					// Pos Y
					>> vecScale[i].x				// Scale X
					>> vecScale[i].y				// Scale Y
					>> vecUIDepth[i]				// UI Depth
					>> vecIsSpriteAnimation[i]		// Is SpriteAnimation
					>> vecFrameSpeed[i]				// Frame Speed
					>> vecRectPosOffset[i].x		// RectPosOffset X
					>> vecRectPosOffset[i].y		// RectPosOffset Y
					>> vecRectScale[i].x			// RectScale X
					>> vecRectScale[i].y;			// RectScale Y
			}

			if (fin.eof())
				break;

			// UIRoot 积己.
			Engine::CGameObject* pRootUI = nullptr;
			pRootUI = CGameUIRoot::Create(m_pGraphicDevice, m_pCommandList,
										  wstrRootObjectTag,
										  wstrDataFilePath,
										  vPos,
										  vScale,
										  bIsSpriteAnimation,
										  fFrameSpeed,
										  vRectPosOffset,
										  vRectScale,
										  UIDepth);
			m_pObjectMgr->Add_GameObject(L"Layer_UI", wstrRootObjectTag, pRootUI);
			m_pHpGaugeRoot = static_cast<CGameUIRoot*>(pRootUI);

			// UIChild 积己.
			Engine::CGameObject* pChildUI = nullptr;
			for (_int i = 0; i < iChildUISize; ++i)
			{
				if (L"BossGaugeFrontV2" == vecObjectTag[i])
				{
					pChildUI = CVergosHpGauge::Create(m_pGraphicDevice, m_pCommandList,
													  wstrRootObjectTag,				// RootObjectTag
													  vecObjectTag[i],					// ObjectTag
													  vecDataFilePath[i],				// DataFilePath
													  vecPos[i],						// Pos
													  vecScale[i],						// Scane
													  (_bool)vecIsSpriteAnimation[i],	// Is Animation
													  vecFrameSpeed[i],					// FrameSpeed
													  vecRectPosOffset[i],				// RectPosOffset
													  vecRectScale[i],					// RectScaleOffset
													  vecUIDepth[i]);						// UI Depth
					m_pHpGauge = static_cast<CVergosHpGauge*>(pChildUI);
				}
				else
				{
					pChildUI = CGameUIChild::Create(m_pGraphicDevice, m_pCommandList,
													wstrRootObjectTag,				// RootObjectTag
													vecObjectTag[i],				// ObjectTag
													vecDataFilePath[i],				// DataFilePath
													vecPos[i],						// Pos
													vecScale[i],					// Scane
													(_bool)vecIsSpriteAnimation[i],	// Is Animation
													vecFrameSpeed[i],				// FrameSpeed
													vecRectPosOffset[i],			// RectPosOffset
													vecRectScale[i],				// RectScaleOffset
													vecUIDepth[i]);					// UI Depth
				}
				m_pObjectMgr->Add_GameObject(L"Layer_UI", vecObjectTag[i], pChildUI);
				static_cast<CGameUIRoot*>(pRootUI)->Add_ChildUI(pChildUI);
			}
		}
	}

	if (nullptr != m_pHpGaugeRoot)
	{
		m_pHpGaugeRoot->Set_IsActive(false);
		m_pHpGaugeRoot->Set_IsChildActive(false);
	}

	return S_OK;
}

void CVergos::Set_ConstantTable()
{
	/*__________________________________________________________________________________________________________
	[ Set ConstantBuffer Data ]
	____________________________________________________________________________________________________________*/
	Engine::SHADOW_DESC tShadowDesc = CShadowLightMgr::Get_Instance()->Get_ShadowDesc();

	Engine::CB_SHADER_MESH tCB_ShaderMesh;
	ZeroMemory(&tCB_ShaderMesh, sizeof(Engine::CB_SHADER_MESH));
	tCB_ShaderMesh.matWorld       = Engine::CShader::Compute_MatrixTranspose(m_pTransCom->m_matWorld);
	tCB_ShaderMesh.matLightView   = Engine::CShader::Compute_MatrixTranspose(tShadowDesc.matLightView);
	tCB_ShaderMesh.matLightProj   = Engine::CShader::Compute_MatrixTranspose(tShadowDesc.matLightProj);
	tCB_ShaderMesh.vLightPos      = tShadowDesc.vLightPosition;
	tCB_ShaderMesh.fLightPorjFar  = tShadowDesc.fLightPorjFar;
	tCB_ShaderMesh.fDissolve      = m_fDissolve;
	tCB_ShaderMesh.vEmissiveColor = m_vEmissiveColor;
	tCB_ShaderMesh.fOffset6 = m_fRedColor;
	m_pShaderCom->Get_UploadBuffer_ShaderMesh()->CopyData(0, tCB_ShaderMesh);
}

void CVergos::Set_ConstantTableShadowDepth()
{
	/*__________________________________________________________________________________________________________
	[ Set ConstantBuffer Data ]
	____________________________________________________________________________________________________________*/
	Engine::SHADOW_DESC tShadowDesc = CShadowLightMgr::Get_Instance()->Get_ShadowDesc();

	Engine::CB_SHADER_SHADOW tCB_ShaderShadow;
	ZeroMemory(&tCB_ShaderShadow, sizeof(Engine::CB_SHADER_SHADOW));
	tCB_ShaderShadow.matWorld = Engine::CShader::Compute_MatrixTranspose(m_pTransCom->m_matWorld);
	tCB_ShaderShadow.matView  = Engine::CShader::Compute_MatrixTranspose(tShadowDesc.matLightView);
	tCB_ShaderShadow.matProj  = Engine::CShader::Compute_MatrixTranspose(tShadowDesc.matLightProj);
	tCB_ShaderShadow.fProjFar = tShadowDesc.fLightPorjFar;

	m_pShadowCom->Get_UploadBuffer_ShaderShadow()->CopyData(0, tCB_ShaderShadow);
}

void CVergos::Set_ConstantTableMiniMap()
{
	m_pTransMiniMap->m_vPos.x = m_pTransCom->m_vPos.x;
	m_pTransMiniMap->m_vPos.z = m_pTransCom->m_vPos.z;
	m_pTransMiniMap->m_vAngle = _vec3(90.0f, 0.0f, 0.0f);
	m_pTransMiniMap->m_vScale = _vec3(6.0f, 6.0f, 6.0f);
	m_pTransMiniMap->Update_Component(0.16f);

	/*__________________________________________________________________________________________________________
	[ Set ConstantBuffer Data ]
	____________________________________________________________________________________________________________*/
	Engine::CB_CAMERA_MATRIX tCB_CameraMatrix;
	ZeroMemory(&tCB_CameraMatrix, sizeof(Engine::CB_CAMERA_MATRIX));
	tCB_CameraMatrix.matView = Engine::CShader::Compute_MatrixTranspose(CShadowLightMgr::Get_Instance()->Get_MiniMapView());
	tCB_CameraMatrix.matProj = Engine::CShader::Compute_MatrixTranspose(CShadowLightMgr::Get_Instance()->Get_MiniMapProj());

	Engine::CB_SHADER_TEXTURE tCB_ShaderTexture;
	ZeroMemory(&tCB_ShaderTexture, sizeof(Engine::CB_SHADER_TEXTURE));
	tCB_ShaderTexture.matWorld	= Engine::CShader::Compute_MatrixTranspose(m_pTransMiniMap->m_matWorld);
	tCB_ShaderTexture.fAlpha    = 1.0f;

	m_pShaderMiniMap->Get_UploadBuffer_CameraTopViewMatrix()->CopyData(0, tCB_CameraMatrix);
	m_pShaderMiniMap->Get_UploadBuffer_ShaderTexture()->CopyData(0, tCB_ShaderTexture);
}

void CVergos::SetUp_AngleInterpolation(const _float& fTimeDelta)
{
	if (m_tAngleInterpolationDesc.is_start_interpolation)
	{
		m_tAngleInterpolationDesc.linear_ratio += m_tAngleInterpolationDesc.interpolation_speed * fTimeDelta;

		m_pTransCom->m_vAngle.y = Engine::LinearInterpolation(m_tAngleInterpolationDesc.v1,
															  m_tAngleInterpolationDesc.v2,
															  m_tAngleInterpolationDesc.linear_ratio);

		if (m_tAngleInterpolationDesc.linear_ratio == Engine::MAX_LINEAR_RATIO)
		{
			m_tAngleInterpolationDesc.is_start_interpolation = false;
		}
	}
}

void CVergos::SetUp_PositionInterpolation(const _float& fTimeDelta)
{
	if (m_tPosInterpolationDesc.is_start_interpolation)
	{
		m_tPosInterpolationDesc.linear_ratio += m_tPosInterpolationDesc.interpolation_speed * fTimeDelta;

		m_pTransCom->m_vPos = Engine::LinearInterpolation(m_tPosInterpolationDesc.v1,
														  m_tPosInterpolationDesc.v2,
														  m_tPosInterpolationDesc.linear_ratio);

		if (m_tPosInterpolationDesc.linear_ratio == Engine::MAX_LINEAR_RATIO)
		{
			m_tPosInterpolationDesc.is_start_interpolation = false;
			m_bIsMoveStop = true;
		}
	}
}

void CVergos::SetUp_Dissolve(const _float& fTimeDelta)
{
	if (m_bIsStartDissolve)
	{
		m_fDissolve += fTimeDelta * 0.33f;

		if (m_fDissolve >= 1.0f)
		{
			m_fDissolve = 1.0f;
			m_bIsReturn = true;
		}
	}
}

void CVergos::Active_Monster(const _float& fTimeDelta)
{
	m_pTransCom->m_vDir = m_pTransCom->Get_LookVector();
	m_pTransCom->m_vDir.Normalize();

	/* Monster MOVE */
	if (!m_bIsMoveStop && m_tPosInterpolationDesc.is_start_interpolation)
	{
		_vec3 vPos = m_pNaviMeshCom->Move_OnNaviMesh(&m_pTransCom->m_vPos,
													 &m_pTransCom->m_vDir,
													 m_pInfoCom->m_fSpeed * fTimeDelta);
		m_pTransCom->m_vPos = vPos;
	}
}

void CVergos::Change_Animation(const _float& fTimeDelta)
{
	if (m_pMeshCom->Is_BlendingComplete())
	{
		switch (m_iMonsterStatus)
		{

		case Vergos::A_WAIT:
		{
			m_bIsCreateCollisionTick = false;
			m_uiAnimIdx = Vergos::A_WAIT;
			m_pMeshCom->Set_AnimationKey(m_uiAnimIdx);
		}
		break;

		case Vergos::A_SPAWN:
		{
			if (nullptr != m_pHpGaugeRoot)
			{
				m_pHpGaugeRoot->Set_IsActive(true);
				m_pHpGaugeRoot->Set_IsChildActive(true);
			}

			m_bIsCreateCollisionTick = false;
			m_uiAnimIdx = Vergos::A_SPAWN;
			m_pMeshCom->Set_AnimationKey(m_uiAnimIdx);

			if (m_pMeshCom->Is_AnimationSetEnd(fTimeDelta))
			{
				m_iMonsterStatus = Vergos::A_WAIT;

				m_uiAnimIdx = Vergos::A_WAIT;
				m_pMeshCom->Set_AnimationKey(m_uiAnimIdx);
			}
		}
		break;

		case Vergos::A_FINCH:
		{
			m_uiAnimIdx = Vergos::A_FINCH;
			m_pMeshCom->Set_AnimationKey(m_uiAnimIdx);

			if (m_pMeshCom->Is_AnimationSetEnd(fTimeDelta))
			{
				m_iMonsterStatus	= Vergos::A_WAIT;

				m_uiAnimIdx			= Vergos::A_WAIT;
				m_pMeshCom->Set_AnimationKey(m_uiAnimIdx);
			}
		}
		break;

		case Vergos::A_SWING_RIGHT:
		{
			m_uiAnimIdx = Vergos::A_SWING_RIGHT;
			m_pMeshCom->Set_AnimationKey(m_uiAnimIdx);

			if (m_pMeshCom->Is_AnimationSetEnd(fTimeDelta))
			{
				m_iMonsterStatus	= Vergos::A_WAIT;

				m_uiAnimIdx			= Vergos::A_WAIT;
				m_pMeshCom->Set_AnimationKey(m_uiAnimIdx);
			}
		}
		break;

		case Vergos::A_SWING_LEFT:
		{
			m_uiAnimIdx = Vergos::A_SWING_LEFT;
			m_pMeshCom->Set_AnimationKey(m_uiAnimIdx);

			if (m_pMeshCom->Is_AnimationSetEnd(fTimeDelta))
			{
				m_iMonsterStatus	= Vergos::A_WAIT;

				m_uiAnimIdx			= Vergos::A_WAIT;
				m_pMeshCom->Set_AnimationKey(m_uiAnimIdx);
			}
		}
		break;

		case Vergos::A_BLOW_LEFT:
		{
			m_uiAnimIdx = Vergos::A_BLOW_LEFT;
			m_pMeshCom->Set_AnimationKey(m_uiAnimIdx);

			if (m_pMeshCom->Is_AnimationSetEnd(fTimeDelta))
			{
				m_iMonsterStatus	= Vergos::A_WAIT;

				m_uiAnimIdx			= Vergos::A_WAIT;
				m_pMeshCom->Set_AnimationKey(m_uiAnimIdx);
			}
		}
		break;

		case Vergos::A_BLOW_RIGHT:
		{
			m_uiAnimIdx = Vergos::A_BLOW_RIGHT;
			m_pMeshCom->Set_AnimationKey(m_uiAnimIdx);

			if (m_pMeshCom->Is_AnimationSetEnd(fTimeDelta))
			{
				m_iMonsterStatus	= Vergos::A_WAIT;

				m_uiAnimIdx			= Vergos::A_WAIT;
				m_pMeshCom->Set_AnimationKey(m_uiAnimIdx);
			}
		}
		break;

		case Vergos::A_BLOW_HEAD:
		{
			m_uiAnimIdx = Vergos::A_BLOW_HEAD;
			m_pMeshCom->Set_AnimationKey(m_uiAnimIdx);

			if (m_pMeshCom->Is_AnimationSetEnd(fTimeDelta))
			{
				m_iMonsterStatus	= Vergos::A_WAIT;

				m_uiAnimIdx			= Vergos::A_WAIT;
				m_pMeshCom->Set_AnimationKey(m_uiAnimIdx);
			}
		}
		break;

		case Vergos::A_BLOW_ROTATION:
		{
			m_uiAnimIdx = Vergos::A_BLOW_ROTATION;
			m_pMeshCom->Set_AnimationKey(m_uiAnimIdx);

			if (m_pMeshCom->Is_AnimationSetEnd(fTimeDelta))
			{
				m_iMonsterStatus	= Vergos::A_WAIT;

				m_uiAnimIdx			= Vergos::A_WAIT;
				m_pMeshCom->Set_AnimationKey(m_uiAnimIdx);
			}
		}
		break;

		case Vergos::A_BREATH_FIRE:
		{
			m_bIsCreateCollisionTick = false;

			m_uiAnimIdx = Vergos::A_BREATH_FIRE;
			m_pMeshCom->Set_AnimationKey(m_uiAnimIdx);

			if (m_pMeshCom->Is_AnimationSetEnd(fTimeDelta))
			{
				m_iMonsterStatus = Vergos::A_WAIT;

				m_uiAnimIdx = Vergos::A_WAIT;
				m_pMeshCom->Set_AnimationKey(m_uiAnimIdx);
			}
		}
		break;

		case Vergos::A_FLY_START:
		{
			m_bIsCreateCollisionTick = false;

			m_uiAnimIdx = Vergos::A_FLY_START;
			m_pMeshCom->Set_AnimationKey(m_uiAnimIdx);

			if (m_pMeshCom->Is_AnimationSetEnd(fTimeDelta))
			{
				m_iMonsterStatus = Vergos::A_FLY_LOOP;

				m_uiAnimIdx = Vergos::A_FLY_LOOP;
				m_pMeshCom->Set_AnimationKey(m_uiAnimIdx);
			}
		}
		break;

		case Vergos::A_FLY_LOOP:
		{
			m_bIsCreateCollisionTick = false;

			m_uiAnimIdx = Vergos::A_FLY_LOOP;
			m_pMeshCom->Set_AnimationKey(m_uiAnimIdx);

			if (m_pMeshCom->Is_AnimationSetEnd(fTimeDelta))
			{
				m_iMonsterStatus = Vergos::A_FLY_END;

				m_uiAnimIdx = Vergos::A_FLY_END;
				m_pMeshCom->Set_AnimationKey(m_uiAnimIdx);
			}
		}
		break;

		case Vergos::A_FLY_END:
		{
			m_bIsCreateCollisionTick = false;

			m_uiAnimIdx = Vergos::A_FLY_END;
			m_pMeshCom->Set_AnimationKey(m_uiAnimIdx);

			if (m_pMeshCom->Is_AnimationSetEnd(fTimeDelta))
			{
				m_iMonsterStatus = Vergos::A_WAIT;

				m_uiAnimIdx = Vergos::A_WAIT;
				m_pMeshCom->Set_AnimationKey(m_uiAnimIdx);
			}
		}
		break;

		case Vergos::A_DEATH:
		{
			m_uiAnimIdx = Vergos::A_DEATH;
			m_pMeshCom->Set_AnimationKey(m_uiAnimIdx);
			
			if (nullptr != m_pHpGaugeRoot)
			{
				m_pHpGaugeRoot->Set_IsActive(false);
				m_pHpGaugeRoot->Set_IsChildActive(false);
			}

			if (m_pMeshCom->Is_AnimationSetEnd(fTimeDelta)) 
			{
				m_bIsStartDissolve = true;

				m_iMonsterStatus = Vergos::A_DEATH;

				m_uiAnimIdx = Vergos::A_DEATH;
				m_pMeshCom->Set_AnimationKey(m_uiAnimIdx);
			}
		}
		break;
		}
	}
}

void CVergos::SetUp_CollisionTick(const _float& fTimeDelta)
{
	
}

void CVergos::SetUp_HpGauge(const _float& fTimeDelta)
{
	if (nullptr != m_pHpGauge && Vergos::A_DEATH != m_iMonsterStatus)
	{
		m_pHpGauge->Set_Percent((_float)(m_pInfoCom->m_iHp) / (_float)(m_pInfoCom->m_iMaxHp), 
								m_pInfoCom->m_iHp, 
								m_pInfoCom->m_iMaxHp);
	}
}

void CVergos::EffectLoop(const _float& fTimeDelta)
{

	if (m_uiAnimIdx == Vergos::BLOW_LEFT || m_uiAnimIdx == Vergos::BLOW_RIGHT)
	{
		string Bone;
		if (m_bisWarningEffect == false)
		{
			m_bisWarningEffect = true;
			
			if (m_uiAnimIdx == Vergos::BLOW_LEFT)
			{
				Bone = "Bip01-L-Hand";
				CEffectMgr::Get_Instance()->Effect_WarningGround(_vec3(390.4, 0.3f, 350.994f), 0.03f);
			}
			else
			{
				Bone = "Bip01-R-Hand";
				CEffectMgr::Get_Instance()->Effect_WarningGround(_vec3(385.4, 0.3f, 351.045f), 0.03f);
			}
		}
		m_fSkillOffset += fTimeDelta;
		if (m_fSkillOffset > 2.f && m_bisDecalEffect == false)
		{
			if (m_uiAnimIdx == Vergos::BLOW_LEFT)
				Bone = "Bip01-L-Hand";
			else
				Bone = "Bip01-R-Hand";

			Engine::HIERARCHY_DESC* pHierarchyDesc = &(m_pMeshCom->Find_HierarchyDesc(Bone));
			_matrix matBoneFinalTransform = (pHierarchyDesc->matScale * pHierarchyDesc->matRotate * pHierarchyDesc->matTrans)
				* pHierarchyDesc->matGlobalTransform;
			_matrix matWorld = matBoneFinalTransform * m_pTransCom->m_matWorld;
			_vec3 Pos= _vec3(matWorld._41, matWorld._42, matWorld._43);
			m_bisDecalEffect = true;
			CEffectMgr::Get_Instance()->Effect_FireDecal(Pos);

			cout <<Bone<< ":"<<Pos.x << "|" << Pos.z << endl;
			CEffectMgr::Get_Instance()->Effect_MeshParticle(L"publicStone" + to_wstring(rand() % 4), _vec3(0.07f), 
				_vec3(0.f), Pos, false, false, 5, 20, 0, 0, 0, _vec2(100, 10), 0, true);
			CEffectMgr::Get_Instance()->Effect_MeshParticle(L"publicStone" + to_wstring(rand() % 4), _vec3(0.07f),
				_vec3(0.f), Pos , false, false, 5, 20, 0, 0, 0, _vec2(100, 10), 0, true);
			CEffectMgr::Get_Instance()->Effect_MeshParticle(L"publicStone" + to_wstring(rand() % 4), _vec3(0.05f),
				_vec3(0.f), Pos, false, false, 5, 20, 0, 0, 0, _vec2(25, 6), 0, true);
			CEffectMgr::Get_Instance()->Effect_MeshParticle(L"publicStone" + to_wstring(rand() % 4), _vec3(0.05f),
				_vec3(0.f), Pos, false, false, 5, 20, 0, 0, 0, _vec2(25, 6), 0, true);
			CEffectMgr::Get_Instance()->Effect_MeshParticle(L"publicStone" + to_wstring(rand() % 4), _vec3(0.02f),
				_vec3(0.f), Pos, false, false, 5, 20, 0, 0, 0, _vec2(15, 2), 0, true);
			CEffectMgr::Get_Instance()->Effect_MeshParticle(L"publicStone" + to_wstring(rand() % 4), _vec3(0.02f),
				_vec3(0.f), Pos, false, false, 5, 20, 0, 0, 0, _vec2(15, 2), 0, true);
		}
	}
	else if (m_uiAnimIdx == Vergos::BLOW_HEAD)
	{
		m_fSkillOffset += fTimeDelta;

		if (m_fSkillOffset > 3.f && m_bisDecalEffect == false)
		{
			m_bisDecalEffect = true;
			Engine::HIERARCHY_DESC* pHierarchyDesc = &(m_pMeshCom->Find_HierarchyDesc("Bip01-Head"));
			_matrix matBoneFinalTransform = (pHierarchyDesc->matScale * pHierarchyDesc->matRotate * pHierarchyDesc->matTrans)
				* pHierarchyDesc->matGlobalTransform;
			_matrix matWorld = matBoneFinalTransform * m_pTransCom->m_matWorld;
			_vec3 Pos = _vec3(matWorld._41, matWorld._42, matWorld._43);
			Pos.y = 0.3f;
			_vec3 Dir = Pos - m_pTransCom->m_vPos;
			Dir.y = 0;
			Dir.Normalize();
			Pos += Dir * 10.f;
			CEffectMgr::Get_Instance()->Effect_MeshParticle(L"publicStone" + to_wstring(rand() % 4), _vec3(0.07f),
				_vec3(0.f), Pos, false, false, 5, 20, 0, 0, 0, _vec2(100, 10), 0, true);
			CEffectMgr::Get_Instance()->Effect_MeshParticle(L"publicStone" + to_wstring(rand() % 4), _vec3(0.07f),
				_vec3(0.f), Pos, false, false, 5, 20, 0, 0, 0, _vec2(100, 10), 0, true);
			CEffectMgr::Get_Instance()->Effect_MeshParticle(L"publicStone" + to_wstring(rand() % 4), _vec3(0.05f),
				_vec3(0.f), Pos, false, false, 5, 20, 0, 0, 0, _vec2(25, 6), 0, true);
			CEffectMgr::Get_Instance()->Effect_MeshParticle(L"publicStone" + to_wstring(rand() % 4), _vec3(0.05f),
				_vec3(0.f), Pos, false, false, 5, 20, 0, 0, 0, _vec2(25, 6), 0, true);
			CEffectMgr::Get_Instance()->Effect_MeshParticle(L"publicStone" + to_wstring(rand() % 4), _vec3(0.02f),
				_vec3(0.f), Pos, false, false, 5, 20, 0, 0, 0, _vec2(15, 2), 0, true);
			CEffectMgr::Get_Instance()->Effect_MeshParticle(L"publicStone" + to_wstring(rand() % 4), _vec3(0.02f),
				_vec3(0.f), Pos, false, false, 5, 20, 0, 0, 0, _vec2(15, 2), 0, true);

			CEffectMgr::Get_Instance()->Effect_RectDecal(Pos, m_pTransCom->m_vAngle.y);
		}
	}
	else if (m_uiAnimIdx == Vergos::A_BREATH_FIRE)
	{
		m_fSkillOffset += fTimeDelta;
		m_bisBreathDelta += fTimeDelta;
		m_fParticleTime += fTimeDelta;
		Engine::HIERARCHY_DESC* pHierarchyDesc = &(m_pMeshCom->Find_HierarchyDesc("FxShot"));
	
	

		if (m_fSkillOffset >3.f)
		{

			m_fBreathTime += fTimeDelta * 0.5f;
			if (m_fParticleTime > 0.1f)
			{
				m_fParticleTime = 0.f;
				_matrix matBoneFinalTransform = (pHierarchyDesc->matScale * pHierarchyDesc->matRotate * pHierarchyDesc->matTrans)
					* pHierarchyDesc->matGlobalTransform;
				_matrix matWorld = matBoneFinalTransform * m_pTransCom->m_matWorld;
				_vec3 Pos = _vec3(matWorld._41, matWorld._42, matWorld._43);
				BreathPos.x = powf((1.f - m_fBreathTime), 2.f) * BazierPos[0].x + 2 * m_fBreathTime * (1 - m_fBreathTime) * BazierPos[1].x
					+ powf(m_fBreathTime, 2.f) * BazierPos[2].x;
				BreathPos.y = 0.f;
				BreathPos.z = powf((1.f - m_fBreathTime), 2.f) * BazierPos[0].z + 2 * m_fBreathTime * (1 - m_fBreathTime) * BazierPos[1].z
					+ powf(m_fBreathTime, 2.f) * BazierPos[2].z;

				CEffectMgr::Get_Instance()->Effect_DirParticle(_vec3(1, 1, 0), _vec3(0.f), m_pTransCom->m_vPos, L"Lighting6", Pos,
					BreathPos, FRAME(1, 1, 1), 2, 20);
			}
			if (m_bisBreathEffect == false)
			{
				m_bisBreathEffect = true;
				m_bisBreathDelta = 0.f;
				CGameObject* pGameObj;
				pGameObj = Pop_Instance(CInstancePoolMgr::Get_Instance()->Get_Effect_Breath_Effect());
				if (nullptr != pGameObj)
				{
					static_cast<CBreathEffect*>(pGameObj)->Set_CreateInfo(_vec3(0.f), _vec3(0.f), _vec3(-65.f, 2.2, -16.f));
					static_cast<CBreathEffect*>(pGameObj)->Set_HierchyDesc(pHierarchyDesc);
					static_cast<CBreathEffect*>(pGameObj)->Set_ParentMatrix(&m_pTransCom->m_matWorld);
					Engine::FAILED_CHECK_RETURN(m_pObjectMgr->Add_GameObject(L"Layer_GameObject", L"Breath", pGameObj), E_FAIL);
				}
			}
			//CEffectMgr::Get_Instance()->Effect_DirParticle(_vec3(1, 1, 0), _vec3(0.f), m_pTransCom->m_vPos, L"Bomb06", Pos,
			//	BreathPos, FRAME(6, 6, 24), 2, 40);
		}
	}
	else
	{
		m_bisBreathEffect = false;
		m_fBreathTime = 0.f;
		m_bisWarningEffect = false;
		m_bisDecalEffect = false;
		m_fSkillOffset = 0.f;
	}
}

Engine::CGameObject* CVergos::Create(ID3D12Device* pGraphicDevice, ID3D12GraphicsCommandList* pCommandList, wstring wstrMeshTag, wstring wstrNaviMeshTag, const _vec3& vScale, const _vec3& vAngle, const _vec3& vPos)
{
	CVergos* pInstance = new CVergos(pGraphicDevice, pCommandList);

	if (FAILED(pInstance->Ready_GameObject(wstrMeshTag, wstrNaviMeshTag, vScale, vAngle, vPos)))
		Engine::Safe_Release(pInstance);

	return pInstance;
}

CVergos** CVergos::Create_InstancePool(ID3D12Device* pGraphicDevice, 
												 ID3D12GraphicsCommandList* pCommandList,
												 const _uint& uiInstanceCnt)
{
	CVergos** ppInstance = new (CVergos * [uiInstanceCnt]);

	for (_uint i = 0; i < uiInstanceCnt; ++i)
	{
		ppInstance[i] = new CVergos(pGraphicDevice, pCommandList);
		ppInstance[i]->m_uiInstanceIdx = i;
		ppInstance[i]->Ready_GameObject(L"Vergos",				// MeshTag
										L"StageBeach_NaviMesh",	// NaviMeshTag
										_vec3(0.05f, 0.05f, 0.05f),	// Scale
										_vec3(0.0f),				// Angle
										_vec3(AWAY_FROM_STAGE));	// Pos
	}

	return ppInstance;
}

void CVergos::Free()
{
	Engine::CGameObject::Free();

	Engine::Safe_Release(m_pMeshCom);
	Engine::Safe_Release(m_pShaderCom);
	Engine::Safe_Release(m_pShadowCom);
	Engine::Safe_Release(m_pColliderSphereCom);
	Engine::Safe_Release(m_pColliderBoxCom);
	Engine::Safe_Release(m_pNaviMeshCom);

	if (nullptr != m_pHpGaugeRoot)
		m_pHpGaugeRoot->Set_DeadGameObject();

	if (nullptr != m_pHpGauge)
		m_pHpGauge->Set_DeadGameObject();
}
