// GateListPage.cpp : implementation file
//

#include "pch.h"
#include "LevelEditor.h"
#include "GateListPage.h"
#include "SheetWithTab.h"
#include "MainFrm.h"

#include "EtcFunction.h"
#include "GLGaeaServer.h"

// CGateListPage dialog

IMPLEMENT_DYNAMIC(CGateListPage, CPropertyPage)
CGateListPage::CGateListPage()
	: CPropertyPage(CGateListPage::IDD)
	, m_CalledPage ( -1 )
	, m_bDlgInit ( FALSE )
	, m_bGateIDInit ( FALSE )
{
}

CGateListPage::~CGateListPage()
{
}

void CGateListPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_GATE, m_listboxGate);
}


BEGIN_MESSAGE_MAP(CGateListPage, CPropertyPage)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_BUTTON_GATE_OK, OnBnClickedButtonGateOk)
	ON_BN_CLICKED(IDC_BUTTON_GATE_CANCEL, OnBnClickedButtonGateCancel)
END_MESSAGE_MAP()


// CGateListPage message handlers
BOOL CGateListPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	m_bDlgInit = TRUE;
	UpdateTreeItem();

	return TRUE;
}

void CGateListPage::OnShowWindow ( BOOL bShow, UINT nStatus )
{
	CPropertyPage::OnShowWindow(bShow, nStatus);

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	if (bShow)
	{
		UpdateTreeItem();
	}
}

void CGateListPage::SetPropertyData ( int nCallPage, DWORD dwGateID )
{
	m_CalledPage = nCallPage;
	m_dwGateID = dwGateID;

	m_bGateIDInit = TRUE;
}

BOOL CGateListPage::UpdateTreeItem()
{
	if ( !m_bDlgInit )
		return FALSE;

	CLevelMainFrame* pFrame = (CLevelMainFrame*)AfxGetMainWnd();
	GLGaeaServer* pGaeaServer = pFrame->GetGLGaeaServer();

	m_listboxGate.ResetContent();

	GLLandMan* pGLLandMan = pGaeaServer->GetRootMap();

	DxLandGateMan* pLandGateMan = &pGLLandMan->GetLandGateMan();
	if (pLandGateMan == NULL)
		return TRUE;

	PDXLANDGATE pCur_Gate = pLandGateMan->GetListLandGate();

	while ( pCur_Gate )
	{
		int nIndex = m_listboxGate.AddString ( pCur_Gate->GetName() );
		m_listboxGate.SetItemData ( nIndex, pCur_Gate->GetGateID() );

		if ( m_bGateIDInit && m_dwGateID == pCur_Gate->GetGateID() )
		{
			if (!(pCur_Gate->GetFlags() & 1))
				continue;

			m_listboxGate.SetCurSel ( nIndex );
		}
		
		pCur_Gate = pCur_Gate->m_pNext;
		
	}

	return TRUE;
}

void CGateListPage::OnBnClickedButtonGateOk()
{
	// TODO: Add your control notification handler code here
	int nIndex = m_listboxGate.GetCurSel ();

	if ( nIndex != -1 )
	{
		DWORD dwGateID = (DWORD)m_listboxGate.GetItemData ( nIndex );

		if ( m_CalledPage == MOBSETPAGE )
			m_pSheetTab->m_MobSetPage.SetPC_GATE_REGEN ( dwGateID );

		m_pSheetTab->SetActivePage ( m_CalledPage );

		m_bGateIDInit = FALSE;
	}
}

void CGateListPage::OnBnClickedButtonGateCancel()
{
	// TODO: Add your control notification handler code here
	m_pSheetTab->SetActivePage ( m_CalledPage );

	m_bGateIDInit = FALSE;
}
