﻿#pragma once


// CTabTexSpriteUV 대화 상자
namespace Engine
{
	class CComponentMgr;
	class CObjectMgr;
	class CManagement;
	class CDescriptorHeapMgr;
}



class CTabTexSpriteUV : public CDialogEx
{
	DECLARE_DYNAMIC(CTabTexSpriteUV)

public:
	CTabTexSpriteUV(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CTabTexSpriteUV();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CTabUI_TexSpriteUV };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

	HRESULT	Ready_TabTexSpriteUV();

public:
	Engine::CManagement*		m_pManagement		= nullptr;
	Engine::CObjectMgr*			m_pObjectMgr		= nullptr;
	Engine::CComponentMgr*		m_pComponentMgr		= nullptr;
	Engine::CDescriptorHeapMgr* m_pDescriptorHeapMgr = nullptr;

	/*__________________________________________________________________________________________________________
	[ Control ]
	____________________________________________________________________________________________________________*/
	afx_msg void	OnNMClickTree2000_TreeTextureTag(NMHDR* pNMHDR, LRESULT* pResult);
	CTreeCtrl		m_TexUITreeCtrl;

	CEdit	m_EditTextureTag;
	CString	m_strTextureTag;

	CEdit	m_EditTextureWidth;
	CEdit	m_EditTexturHeight;
	int		m_iTextureWidth;
	int		m_iTextureHeight;

	afx_msg void	OnLbnSelchangeList2000_TextureIndex();
	CListBox		m_ListBoxTexIndex;
	_uint			m_iSelectTexIndex = 0;
};
