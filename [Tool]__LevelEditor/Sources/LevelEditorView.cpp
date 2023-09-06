// LevelEditorView.cpp : implementation of the CLevelEditorView class
//

#include "pch.h"

#include "DxResponseMan.h"

#include "LevelEditor.h"
#include "LevelEditorDoc.h"
#include "LevelEditorView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern BOOL				g_bFRAME_LIMIT;

// CLevelEditorView
// BOOL CLevelEditorView::m_bRENDQUAD = FALSE;

IMPLEMENT_DYNCREATE(CLevelEditorView, CView)

BEGIN_MESSAGE_MAP(CLevelEditorView, CView)
END_MESSAGE_MAP()

// CLevelEditorView construction/destruction

CLevelEditorView::CLevelEditorView()
	: CD3DApplication()
	, m_bCreated ( FALSE )
	, m_p3dPosMesh ( NULL )
	, m_pDxEditmat ( NULL )
	, m_vCollisionPos ( 0.f, 0.f, 0.f )
	, m_spDxLandMan( new DxLandMan )
{
	// TODO: add construction code here
	m_bUseDepthBuffer	= TRUE;

	g_bFRAME_LIMIT = TRUE;
}

CLevelEditorView::~CLevelEditorView()
{
}

BOOL CLevelEditorView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CLevelEditorView drawing

void CLevelEditorView::OnDraw(CDC* /*pDC*/)
{
	CLevelEditorDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
}


// CLevelEditorView diagnostics

#ifdef _DEBUG
void CLevelEditorView::AssertValid() const
{
	CView::AssertValid();
}

void CLevelEditorView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CLevelEditorDoc* CLevelEditorView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CLevelEditorDoc)));
	return (CLevelEditorDoc*)m_pDocument;
}
#endif //_DEBUG


// CLevelEditorView message handlers
void CLevelEditorView::OnInitialUpdate()
{
	__super::OnInitialUpdate();

	CD3DApplication::Create(CView::m_hWnd, CView::m_hWnd, AfxGetInstanceHandle());
}

void CLevelEditorView::PostNcDestroy()
{
	CD3DApplication::Cleanup3DEnvironment();

	__super::PostNcDestroy();
}

void CLevelEditorView::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	if (m_bCreated) ResizeWindow(cx, cy);
}

void CLevelEditorView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	CLevelEditorApp* pApp = (CLevelEditorApp*)AfxGetApp();
	pApp->SetActive(bActivate);

	__super::OnActivateView(bActivate, pActivateView, pDeactiveView);
}
