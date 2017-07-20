// huma.h : main header file for the HUMA application
//

#if !defined(AFX_HUMA_H__274E36E6_E064_49FF_B730_E7C905887172__INCLUDED_)
#define AFX_HUMA_H__274E36E6_E064_49FF_B730_E7C905887172__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CHumaApp:
// See huma.cpp for the implementation of this class
//

class CHumaApp : public CWinApp
{
public:
	CHumaApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHumaApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CHumaApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HUMA_H__274E36E6_E064_49FF_B730_E7C905887172__INCLUDED_)
