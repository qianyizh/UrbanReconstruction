// humaDoc.cpp : implementation of the CHumaDoc class
//

#include "stdafx.h"
#include "huma.h"

#include "humaDoc.h"
#include "humaview.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CHumaDoc

IMPLEMENT_DYNCREATE(CHumaDoc, CDocument)

BEGIN_MESSAGE_MAP(CHumaDoc, CDocument)
	//{{AFX_MSG_MAP(CHumaDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHumaDoc construction/destruction

CHumaDoc::CHumaDoc()
{
	// TODO: add one-time construction code here
	m_pMesh=NULL;
	m_pTempMesh=NULL;

}

CHumaDoc::~CHumaDoc()
{
	if(m_pMesh!=NULL)
		delete m_pMesh;
	if(m_pTempMesh!=NULL)
		delete m_pTempMesh;
}

BOOL CHumaDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CHumaDoc serialization

void CHumaDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CHumaDoc diagnostics

#ifdef _DEBUG
void CHumaDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CHumaDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CHumaDoc commands

BOOL CHumaDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;
	
	// TODO: Add your specialized creation code here
	m_pMesh=new CMesh();
	m_pMesh->Load(lpszPathName);
	POSITION pos = GetFirstViewPosition();
	CHumaView* pView = (CHumaView*)GetNextView(pos);
	pView->m_scene.AddMesh(m_pMesh);
	
	return TRUE;
}

BOOL CHumaDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{
	return m_pMesh->Save(lpszPathName);
//	return CDocument::OnSaveDocument(lpszPathName);
}
