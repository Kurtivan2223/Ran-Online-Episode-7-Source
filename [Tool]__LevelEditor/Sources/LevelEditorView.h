// LevelEditorView.h : interface of the CLevelEditorView class
//


#pragma once

#include "resource.h"

#include "d3dapp.h"

#include "DxMethods.h"
#include "DxEditBox.h"
#include "DxObjectMRS.h"
#include "DxLandMan.h"
#include "GLLandMan.h"
#include "GLGaeaServer.h"

class DxFrameMesh;
class DxEditMat;

class CLevelEditorDoc;

enum CUR_TYPE
{
	CUR_SEL_MOBSCH		= ID_EDIT_MENU_SELECT_MOBSCH,
	CUR_NONE			= ID_EDIT_MENU_NONE,
	CUR_SELECT			= ID_EDIT_MENU_SELECT,
	CUR_MOVE			= ID_EDIT_MENU_MOVE,
	CUR_ROTATE			= ID_EDIT_MENU_ROTATE,
	CUR_SCALE			= ID_EDIT_MENU_SCALE,
	CUR_EDIT_BOX		= ID_EDIT_MENU_EDITBOX,
};

class CLevelEditorView : public CView, public CD3DApplication
{
protected: // create from serialization only
	CLevelEditorView();
	DECLARE_DYNCREATE(CLevelEditorView)

// Attributes
public:
	CLevelEditorDoc* GetDocument() const;

public:
	static BOOL				m_bRENDQUAD;
	shared_ptr<DxLandMan>	m_spDxLandMan;

	BOOL					m_bCreated;

	DxFrameMesh*			m_p3dPosMesh;
	DxEditMat*				m_pDxEditmat;

	DxEditBox				m_EditBox;
	DxObjectMRS				m_sObjectMRS;

	D3DXVECTOR3				m_vCollisionPos;			

protected:
	static CUR_TYPE			m_CurType;

// Operations
public:
	virtual void OnInitialUpdate();

	void	ActiveEditBox ( D3DXVECTOR3 &vMax, D3DXVECTOR3 &vMin );
	void	ActiveEditMatrix ( DXAFFINEMATRIX *pAffineMatrix );
	void	ActiveEditMatrix2 ( D3DXMATRIX *matWorld );

	void	DeActiveEditBox();
	void	DeActiveEditMatrix ();

	void	CurMove();
	void	CurSelect();

	void	SetCurType ( CUR_TYPE cType ) { m_CurType = cType; }
	void	SetCurEditType ( DWORD EditType );

	void	SetCharMoveTo();

	void	SetActive ( BOOL bActive );

	D3DXVECTOR3*	GetCollisionPos() { return &m_vCollisionPos; }
	DxEditBox*		GetEditBox() { return &m_EditBox; }
	DxLandMan*		GetLandMan() { return m_spDxLandMan.get(); }
	shared_ptr<DxLandMan>	GetspLandMan() { return m_spDxLandMan; }

	LPDIRECT3DDEVICEQ GetD3DDevice () { return m_pd3dDevice; }
	D3DCAPSQ GetD3dCaps() { return m_d3dCaps; }

	BOOL	SaveGLLandMan ( const char* szFile );
	BOOL	LoadGLLandMan ( const char* szFile );

private:
	HRESULT ConfirmDevice( D3DCAPSQ*,DWORD,D3DFORMAT );
	HRESULT OneTimeSceneInit();
	HRESULT CreateObjects();
	HRESULT InitDeviceObjects();	
	HRESULT RestoreDeviceObjects();
	HRESULT FrameMove();
	HRESULT Render();
	HRESULT InvalidateDeviceObjects();
	HRESULT DeleteDeviceObjects();
	HRESULT FinalCleanup();

	HRESULT RenderText();
	void	RenderLandEffectState();

	HRESULT ResizeWindow ( int cx, int cy )
	{
		if ( cx < 10 )	cx = 10;
		if ( cy < 10 )	cy = 10;

		m_d3dpp.BackBufferWidth  = cx;
		m_d3dpp.BackBufferHeight = cy;

		return Resize3DEnvironment();
	}

protected:
	virtual void PostNcDestroy();

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	virtual HRESULT FrameMove3DEnvironment();
	virtual HRESULT Render3DEnvironment();

protected:

// Implementation
public:
	virtual ~CLevelEditorView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

#ifndef _DEBUG  // debug version in LevelEditorView.cpp
inline CLevelEditorDoc* CLevelEditorView::GetDocument() const
   { return reinterpret_cast<CLevelEditorDoc*>(m_pDocument); }
#endif

