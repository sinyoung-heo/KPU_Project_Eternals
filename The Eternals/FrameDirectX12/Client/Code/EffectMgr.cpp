#include "stdafx.h"
#include "EffectMgr.h"
#include "IceStorm.h"
#include "IceDecal.h"
#include "FireDecal.h"
#include "SwordEffect.h"
#include "ObjectMgr.h"
#include "GraphicDevice.h"
#include "FrameMesh.h"
#include "FireRing.h"
#include "TextureEffect.h"
#include "MagicCircle.h"
#include "ParticleEffect.h"
#include "PublicSphere.h"
#include "PublicPlane.h"
#include "MagicCircleGlow.h"
#include "GridShieldEffect.h"
#include "EffectShield.h"
#include "EffectAxe.h"
#include "SwordEffect_s.h"
#include "IceStorm_m.h"
#include "IceStorm_s.h"
#include "DistTrail.h"
IMPLEMENT_SINGLETON(CEffectMgr)

CEffectMgr::CEffectMgr()
{
	m_pGraphicDevice=Engine::CGraphicDevice::Get_Instance()->Get_GraphicDevice();
	m_pCommandList=Engine::CGraphicDevice::Get_Instance()->Get_CommandList(Engine::CMDID::CMD_MAIN);
	m_pObjectMgr= Engine::CObjectMgr::Get_Instance();
}

void CEffectMgr::Effect_Dust(_vec3 vecPos, float Radius)
{
	_vec3 newPos;
	for (int i = 0; i < 18; i++)
	{
		newPos.y = 0.5f;
		newPos.x = vecPos.x + Radius * cos(XMConvertToRadians(i * 20.f));
		newPos.z = vecPos.z + Radius * sin(XMConvertToRadians(i * 20.f));
		Effect_TextureEffect(L"Dust", _vec3(3.f), _vec3(0.f), newPos, FRAME(12, 7, 84.0f), false, false);
	
	}
}

void CEffectMgr::Effect_IceStorm(_vec3 vecPos , int Cnt , float Radius )
{
	for (int i = 0; i < 36; i+=(36/Cnt))
	{
		pGameObj = Pop_Instance(CInstancePoolMgr::Get_Instance()->Get_Effect_IceStormEffect());
		if (nullptr != pGameObj)
		{
			static_cast<CIceStorm*>(pGameObj)->Set_CreateInfo(_vec3(0.f), _vec3(0.f), vecPos, Radius, XMConvertToRadians(i * 10.f));
			Engine::FAILED_CHECK_RETURN(m_pObjectMgr->Add_GameObject(L"Layer_GameObject", L"IceStorm1", pGameObj), E_FAIL);
		}

	}
}

void CEffectMgr::Effect_IceStorm_s(_vec3 vecPos, float Radius)
{
	pGameObj = Pop_Instance(CInstancePoolMgr::Get_Instance()->Get_Effect_IceStorm_s_Effect());
	if (nullptr != pGameObj)
	{
		static_cast<CIceStorm_s*>(pGameObj)->Set_CreateInfo(_vec3(0.f), _vec3(0.f), vecPos, Radius, XMConvertToRadians(rand() % 36 * 10.f));
		Engine::FAILED_CHECK_RETURN(m_pObjectMgr->Add_GameObject(L"Layer_GameObject", L"IceStorm1", pGameObj), E_FAIL);
	}
}

void CEffectMgr::Effect_SwordEffect(_vec3 vecPos,_vec3 vecDir)
{
	vecPos.y += 100.f;
	pGameObj = Pop_Instance(CInstancePoolMgr::Get_Instance()->Get_Effect_SwordEffect());
	if (nullptr != pGameObj)
	{
		static_cast<CSwordEffect*>(pGameObj)->Set_CreateInfo(_vec3(0.f), _vec3(0.f, 0.f, 180.f), vecPos + vecDir * 5);
		Engine::FAILED_CHECK_RETURN(m_pObjectMgr->Add_GameObject(L"Layer_GameObject", L"SwordEffect", pGameObj), E_FAIL);
	}
}

void CEffectMgr::Effect_SwordEffect_s(_vec3 vecPos, _vec3 vecDir)
{
	_vec3 upVec = _vec3(0, 1, 0);
	_vec3 crossVec = upVec.Cross_InputV1(vecDir);

	for (int i = -6; i <= 6; i+=3)
	{
		_vec3 newPos = vecPos - crossVec * (1.5f) * i;
		newPos -= (vecDir * (18 - abs( i)));
		pGameObj = Pop_Instance(CInstancePoolMgr::Get_Instance()->Get_Effect_Sword_s_Effect());
		if (nullptr != pGameObj)
		{	
			static_cast<CSwordEffect_s*>(pGameObj)->Set_CreateInfo(_vec3(0.f), _vec3(0.f), newPos,vecDir);
			static_cast<CSwordEffect_s*>(pGameObj)->Set_isScaleAnim(true);
			Engine::FAILED_CHECK_RETURN(m_pObjectMgr->Add_GameObject(L"Layer_GameObject", L"publicSword", pGameObj), E_FAIL);
		}
		
	}
}

void CEffectMgr::Effect_Straight_IceStorm(_vec3 vecPos, _vec3 vecDir)
{
	_matrix rotationY=XMMatrixRotationY(XMConvertToRadians(20.f));
	XMVECTOR temp= XMVector3TransformCoord(vecDir.Get_XMVECTOR(), rotationY);
	_vec3 DirectVec;
	XMStoreFloat3(&DirectVec, temp);
	DirectVec.Normalize();
	float fPlayerAngleY = m_pObjectMgr->Get_GameObject(L"Layer_GameObject", L"ThisPlayer")->Get_Transform()->m_vAngle.y;
	for (int i = 0; i < 13; i++)
	{
		_vec3 PosOffSet = vecDir * 2.f + (DirectVec *(1+ i)*(1+powf(i/13,2)));
		_vec3 newAngle = _vec3(rand() % 20 - 10, fPlayerAngleY, rand() % 20 - 10);
		pGameObj = Pop_Instance(CInstancePoolMgr::Get_Instance()->Get_Effect_IceStorm_m_Effect());
		if (nullptr != pGameObj)
		{
			static_cast<CIceStorm_m*>(pGameObj)->Set_CreateInfo(_vec3(0.f), newAngle, vecPos + PosOffSet, 0.05f + (0.02f * i));
			Engine::FAILED_CHECK_RETURN(m_pObjectMgr->Add_GameObject(L"Layer_GameObject", L"publicSkill3", pGameObj), E_FAIL);
		}
		CEffectMgr::Get_Instance()->Effect_Particle(vecPos + PosOffSet, 5, L"Lighting0", _vec3(0.3f));
	}
}

void CEffectMgr::Effect_FireDecal(_vec3 vecPos)
{
	pGameObj = CFireDecal::Create(m_pGraphicDevice, m_pCommandList,
		L"PublicPlane00",
		_vec3(0.01f),
		_vec3(0.f, 0.0f, 0.0f),
		vecPos);
	Engine::FAILED_CHECK_RETURN(m_pObjectMgr->Add_GameObject(L"Layer_GameObject", L"FireDecal", pGameObj), E_FAIL);
}
void CEffectMgr::Effect_IceDecal(_vec3 vecPos)
{
	pGameObj = Pop_Instance(CInstancePoolMgr::Get_Instance()->Get_Effect_IceDecal_Effect());
	if (nullptr != pGameObj)
	{
		static_cast<CIceDecal*>(pGameObj)->Set_CreateInfo(_vec3(0.01f),_vec3(0.f, 0.0f, 0.0f),vecPos);
		Engine::FAILED_CHECK_RETURN(m_pObjectMgr->Add_GameObject(L"Layer_GameObject", L"IceDecal", pGameObj), E_FAIL);
	}
}

void CEffectMgr::Effect_ArrowHitted(_vec3 vecPos)
{	
	Effect_TextureEffect(L"Lighting3", _vec3(0.f), _vec3(0.f), vecPos, FRAME(4, 4, 16.0f), true, true,
		_vec4(0.f, 0.3f, 0.6f, 1.f));
	
}

void CEffectMgr::Effect_FireCone(_vec3 vecPos, float RotY , _vec3 vecDir)
{
	vecPos.y = 0.5f;

	pGameObj = Pop_Instance(CInstancePoolMgr::Get_Instance()->Get_Effect_FireRing_Effect());
	if (nullptr != pGameObj)
	{
		static_cast<CFireRing*>(pGameObj)->Set_CreateInfo(_vec3(0.01f), _vec3(0.f, 0.0f, 0.0f), vecPos);
		Engine::FAILED_CHECK_RETURN(m_pObjectMgr->Add_GameObject(L"Layer_GameObject", L"HalfMoon0", pGameObj), E_FAIL);
	}

	for (int i = 0; i < 5; i++)
	{
		pGameObj = Pop_Instance(CInstancePoolMgr::Get_Instance()->Get_Effect_FrameMesh_Effect());
		if (nullptr != pGameObj)
		{
			static_cast<CFrameMesh*>(pGameObj)->Set_CreateInfo(_vec3(0.015f, 20.5, 0.015), _vec3(-0.f, RotY + 60 * i, -0.f)
				, vecPos, FRAME(8, 8, 128));
			Engine::FAILED_CHECK_RETURN(m_pObjectMgr->Add_GameObject(L"Layer_GameObject", L"PublicCylinder02", pGameObj), E_FAIL);
		}
	}


}

void CEffectMgr::Effect_Test(_vec3 vecPos)
{
	
}

void CEffectMgr::Effect_GridShieldEffect(_vec3 vecPos,int type,Engine::CTransform* parentTransform)
{
	int Pipeidx = 0;
	_vec3 Texidx;
	vecPos.y = 1.f + rand()%10 * 0.01;
	type == 0 ? Pipeidx = 2 : Pipeidx=10;
	type == 0 ? Texidx = _vec3(0,2,16) : Texidx=_vec3(0, 25, 16);
	//pGameObj = CGridShieldEffect::Create(m_pGraphicDevice, m_pCommandList,
	//	L"PublicSphere00",
	//	_vec3(0.00f),
	//	_vec3(0.f, 0.f, 0.f),
	//	vecPos, Pipeidx
	//);
	//Engine::FAILED_CHECK_RETURN(m_pObjectMgr->Add_GameObject(L"Layer_GameObject", L"PublicSphere00", pGameObj), E_FAIL);

	pGameObj = Pop_Instance(CInstancePoolMgr::Get_Instance()->Get_Effect_GridShieldEffect());
	if (nullptr != pGameObj)
	{
		static_cast<CGridShieldEffect*>(pGameObj)->Set_CreateInfo(_vec3(0.f), _vec3(0.f), vecPos, Texidx.x, Texidx.y, Texidx.z,Pipeidx, true, true, parentTransform);
		Engine::FAILED_CHECK_RETURN(m_pObjectMgr->Add_GameObject(L"Layer_GameObject", L"PublicSphere00", pGameObj), E_FAIL);
	}
	/*if (type == 0)
		static_cast<CGridShieldEffect*>(pGameObj)->Set_TexIDX(0, 2, 16);
	else
		static_cast<CGridShieldEffect*>(pGameObj)->Set_TexIDX(0, 25, 16);*/


	if (type == 0)
		Effect_MagicCircle_Effect(_vec3(0.0f), _vec3(0.0f), vecPos, 18, 18, 2,true, true, parentTransform, true);
	else
		Effect_MagicCircle_Effect(_vec3(0.0f), _vec3(0.0f), vecPos, 19, 19, 2, true, true, parentTransform, true);
}

void CEffectMgr::Effect_Shield(_vec3 vecPos,Engine::CTransform* parentTransform)
{
	for (int i = 0; i < 5; i++)
	{
		vecPos.y = 1.f;
		pGameObj = Pop_Instance(CInstancePoolMgr::Get_Instance()->Get_Effect_ShieldEffect());
		if (nullptr != pGameObj)
		{
			static_cast<CEffectShield*>(pGameObj)->Set_CreateInfo(_vec3(0.0f), _vec3(0.0f), vecPos,
				5.5f,0.12f,3.f,(360 / 5) * i, parentTransform);
			Engine::FAILED_CHECK_RETURN(m_pObjectMgr->Add_GameObject(L"Layer_GameObject", L"publicShield", pGameObj), E_FAIL);
		}
	}
	vecPos.y = rand() % 50 * 0.01f;
	vecPos.z += 0.2f;
	Effect_MagicCircle_Effect(_vec3(0.0f), _vec3(0.0f), vecPos, 20, 20, 2, true, true, parentTransform, true);
}

void CEffectMgr::Effect_Axe(_vec3 vecPos, Engine::CTransform* parentTransform)
{
	for (int i = 0; i < 5; i++)
	{
		vecPos.y = -1.f;
		pGameObj = Pop_Instance(CInstancePoolMgr::Get_Instance()->Get_Effect_AxeEffect());
		if (nullptr != pGameObj)
		{
			static_cast<CEffectAxe*>(pGameObj)->Set_CreateInfo(_vec3(0.0f), _vec3(0.0f), vecPos, (360 / 5) * i, parentTransform);
			Engine::FAILED_CHECK_RETURN(m_pObjectMgr->Add_GameObject(L"Layer_GameObject", L"publicAxe", pGameObj), E_FAIL);
		}
	}
	vecPos.y = rand() % 50 * 0.01f;
	vecPos.z += 0.2f;
	Effect_MagicCircle_Effect(_vec3(0.0f), _vec3(0.0f), vecPos, 21, 21, 2, true, true, parentTransform, true);
}

void CEffectMgr::Effect_TargetShield(_vec3 vecPos, Engine::CTransform* parentTransform)
{
	for (int i = 0; i < 3; i++)
	{
		vecPos.y = 1.f;
		pGameObj = Pop_Instance(CInstancePoolMgr::Get_Instance()->Get_Effect_ShieldEffect());
		if (nullptr != pGameObj)
		{
			static_cast<CEffectShield*>(pGameObj)->Set_CreateInfo(_vec3(0.0f), _vec3(0.0f), vecPos,
				180.f, 0.06f, 1.5f
				, (360 / 3) * i, parentTransform,true);
			Engine::FAILED_CHECK_RETURN(m_pObjectMgr->Add_GameObject(L"Layer_GameObject", L"publicShield", pGameObj), E_FAIL);
		}
	}
}

void CEffectMgr::Effect_TargetAxe(_vec3 vecPos, Engine::CTransform* parentTransform)
{
	for (int i = 0; i < 3; i++)
	{
		vecPos.y = -1.f;
		pGameObj = Pop_Instance(CInstancePoolMgr::Get_Instance()->Get_Effect_AxeEffect());
		if (nullptr != pGameObj)
		{
			static_cast<CEffectAxe*>(pGameObj)->Set_CreateInfo(_vec3(0.0f), _vec3(0.0f), vecPos, (360 / 3) * i, parentTransform);
			Engine::FAILED_CHECK_RETURN(m_pObjectMgr->Add_GameObject(L"Layer_GameObject", L"publicAxe", pGameObj), E_FAIL);
		}
	}
}

void CEffectMgr::Effect_DistTrail(_vec3 vecPos, _vec3 Angle,bool isCrossFilter,float SizeOffSet)
{
	pGameObj = Pop_Instance(CInstancePoolMgr::Get_Instance()->Get_Effect_DistTrail_Effect());
	if (nullptr != pGameObj)
	{
		static_cast<CDistTrail*>(pGameObj)->Set_CreateInfo(_vec3(0.01f), Angle, vecPos);
		Engine::FAILED_CHECK_RETURN(m_pObjectMgr->Add_GameObject(L"Layer_GameObject", L"DistTrail", pGameObj), E_FAIL);
		static_cast<CDistTrail*>(pGameObj)->Set_IsCrossFilter(isCrossFilter);
		static_cast<CDistTrail*>(pGameObj)->Set_SizeOffset(SizeOffSet);
		
	}
}

void CEffectMgr::Effect_Particle(_vec3 vecPos, _int Cnt, wstring Tag,_vec3 vecScale,FRAME Frame)
{
	//Snow Lighting1 Lighting0
	Engine::CGameObject *particleobj = Pop_Instance(CInstancePoolMgr::Get_Instance()->Get_Effect_Particle_Effect());
	if (nullptr != particleobj)
	{
		static_cast<CParticleEffect*>(particleobj)->Set_CreateInfo(vecScale, _vec3(0.f), vecPos,Tag, Frame
		,9,Cnt);
		Engine::FAILED_CHECK_RETURN(m_pObjectMgr->Add_GameObject(L"Layer_GameObject",Tag, particleobj), E_FAIL);
	}
}

void CEffectMgr::Effect_TextureEffect(wstring TexTag, _vec3 Scale, _vec3 Angle, _vec3 Pos, FRAME frame, bool isLoop, bool isScaleAnim, _vec4 colorOffset
 , bool isFollowHand , Engine::HIERARCHY_DESC* hierachy, Engine::CTransform* parentTransform)
{
	Engine::CGameObject* textureObj = Pop_Instance(CInstancePoolMgr::Get_Instance()->Get_Effect_TextureEffect());
	if (nullptr != textureObj)
	{
		static_cast<CTextureEffect*>(textureObj)->Set_CreateInfo(TexTag,Scale,Angle,Pos,frame,isLoop,isScaleAnim,colorOffset
		, isFollowHand, hierachy, parentTransform);
		Engine::FAILED_CHECK_RETURN(m_pObjectMgr->Add_GameObject(L"Layer_GameObject", TexTag, textureObj), E_FAIL);

	}
}

void CEffectMgr::Effect_MagicCircle_Effect(const _vec3& vScale, const _vec3& vAngle, const _vec3& vPos, _int Diff, _int Norm, _int Spec
	, bool bisRotate, bool bisScaleAnim, const Engine::CTransform* ParentTransform, bool bisFollowPlayer)
{
	Engine::CGameObject* MagicCircleObj = Pop_Instance(CInstancePoolMgr::Get_Instance()->Get_Effect_MagicCircleEffect());
	if (nullptr != MagicCircleObj)
	{
		static_cast<CMagicCircle*>(MagicCircleObj)->Set_CreateInfo(vScale, vAngle, vPos, bisRotate, bisScaleAnim, ParentTransform,
			bisFollowPlayer);
		static_cast<CMagicCircle*>(MagicCircleObj)->Set_TexIDX(Diff, Norm, Spec);
		Engine::FAILED_CHECK_RETURN(m_pObjectMgr->Add_GameObject(L"Layer_GameObject",L"MagicCircle", MagicCircleObj), E_FAIL);
	}
}



void CEffectMgr::Free(void)
{
}
