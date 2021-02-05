#pragma once
#include "GameObject.h"

BEGIN(Engine)

class CShaderColor;
class CShaderTexture;
class CShaderMesh;
class CShaderSkyBox;
class CShaderShadow;
class CShaderSSAO;
class ENGINE_DLL CCamera : public CGameObject
{
protected:
	explicit CCamera(ID3D12Device* pGraphicDevice, ID3D12GraphicsCommandList* pCommandList);
	explicit CCamera(const CCamera& rhs);
	virtual ~CCamera() = default;

public:
	// Get
	const CAMERA_DESC&	Get_CameraInfo()								{ return m_tCameraInfo; }
	const PROJ_DESC&	Get_ProjInfo()									{ return m_tProjInfo; }
	const ORTHO_DESC&	Get_OrthInfo()									{ return m_tOrthoInfo; }

	// Set
	void				Set_CameraInfo(const CAMERA_DESC& tCameraInfo)	{ m_tCameraInfo = tCameraInfo; }
	void				Set_Eye(const _vec3& vEye)						{ m_tCameraInfo.vEye = vEye; }
	void				Set_At(const _vec3& vAt)						{ m_tCameraInfo.vEye = vAt; }
	void				Set_Up(const _vec3& vUp)						{ m_tCameraInfo.vEye = vUp; }
	void				Set_ProjInfo(const PROJ_DESC& tProjInfo)		{ m_tProjInfo = tProjInfo; }
	void				Set_OrthoInfo(const ORTHO_DESC& tOrthoInfo)		{ m_tOrthoInfo = tOrthoInfo; }

	// CGameObject을(를) 통해 상속됨
	virtual HRESULT		Ready_GameObject(const CAMERA_DESC& tCameraInfo,
										 const PROJ_DESC& tProjInfo,
										 const ORTHO_DESC& tOrthoInfo);
	virtual _int		Update_GameObject(const _float& fTimeDelta);

protected:
	void				Set_ConstantTable();

protected:
	/*__________________________________________________________________________________________________________
	[ Value ]
	____________________________________________________________________________________________________________*/
	CAMERA_DESC	m_tCameraInfo	{ };	// Camera - Eyte, At, Up
	PROJ_DESC	m_tProjInfo		{ };	// 원근 투영 정보를 담은 구조체.
	ORTHO_DESC	m_tOrthoInfo	{ };	// 직교 투영 정보를 담은 구조체.

	/*__________________________________________________________________________________________________________
	[ Shader Component ]
	____________________________________________________________________________________________________________*/
	CShaderColor*	m_pShaderColor		= nullptr;
	CShaderTexture* m_pShaderTexture	= nullptr;
	CShaderSkyBox*	m_pShaderSkyBox		= nullptr;
	CShaderMesh*	m_pShaderMesh		= nullptr;
	CShaderSSAO* m_pShaderSSAO = nullptr;

protected:
	virtual void Free();
};

END