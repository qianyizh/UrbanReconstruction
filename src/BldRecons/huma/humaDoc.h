// humaDoc.h : interface of the CHumaDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_HUMADOC_H__5C5B097B_351F_4EEC_B830_9A643EA0BBE8__INCLUDED_)
#define AFX_HUMADOC_H__5C5B097B_351F_4EEC_B830_9A643EA0BBE8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "model\mesh.h"

class CHumaDoc : public CDocument
{
protected: // create from serialization only
	CHumaDoc();
	DECLARE_DYNCREATE(CHumaDoc)

// Attributes
public:
	CMesh*		m_pMesh;//temp
	CMesh*		m_pTempMesh;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHumaDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CHumaDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CHumaDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HUMADOC_H__5C5B097B_351F_4EEC_B830_9A643EA0BBE8__INCLUDED_)
