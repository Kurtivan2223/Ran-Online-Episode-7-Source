#pragma once

#include "resource.h"

#include "GLCrowData.h"

// CCrowTree dialog
class CsheetWithTab;

class CCrowTree : public CPropertyPage
{
	DECLARE_DYNAMIC(CCrowTree)

public:
	CCrowTree();
	virtual ~CCrowTree();

// Dialog Data
	enum { IDD = IDD_CROW_TREE_PAGE };

protected:
	CsheetWithTab* m_pSheetTab;
	BOOL			m_bDlgInit;

public:
	BOOL			m_bNativeIDInit;
	int				m_CalledPage;
	SNATIVEID		m_NativeID;

public:
	void	SetSheetTab ( CsheetWithTab* SheetTab ) { m_pSheetTab = SheetTab; }
	void	SetPropertyData ( int nCallPage, SNATIVEID NativeID );
	BOOL	UpdateTreeItem();
	void	CleanAllItem();

protected:
	virtual BOOL OnInitDialog();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	CTreeCtrl	m_ctrlItemTree;
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnBnClickedButtonCrowOk();
	afx_msg void OnBnClickedButtonCrowCancel();
};
