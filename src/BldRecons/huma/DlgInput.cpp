// DlgInput.cpp : implementation file
//

#include "stdafx.h"
#include "huma.h"
#include "DlgInput.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgInput dialog


CDlgInput::CDlgInput(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgInput::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgInput)
	m_InputNumber = 100;
	m_bInfinite = TRUE;
	//}}AFX_DATA_INIT
}


void CDlgInput::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgInput)
	DDX_Text(pDX, IDC_EDIT1, m_InputNumber);
	DDV_MinMaxUInt(pDX, m_InputNumber, 1, 5000);
	DDX_Check(pDX, IDC_CHECK_INFINITE, m_bInfinite);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgInput, CDialog)
	//{{AFX_MSG_MAP(CDlgInput)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgInput message handlers

void CDlgInput::OnOK() 
{
	// TODO: Add extra validation here
	UpdateData();
	CDialog::OnOK();
}

void CDlgInput::OnCancel() 
{
	// TODO: Add extra cleanup here
	CDialog::OnCancel();
}
