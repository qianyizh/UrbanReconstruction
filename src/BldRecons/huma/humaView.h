// humaView.h : interface of the CHumaView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_HUMAVIEW_H__6E6339AE_343E_4E93_832B_BC5EFE7C97AD__INCLUDED_)
#define AFX_HUMAVIEW_H__6E6339AE_343E_4E93_832B_BC5EFE7C97AD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "display\scene.h"
#include "dlginput.h"

class CHumaView : public CView
{
protected: // create from serialization only
	CHumaView();
	DECLARE_DYNCREATE(CHumaView)

// Attributes
public:
	CHumaDoc* GetDocument();

private:
	void InitUI();

// Operations
public:
	CPoint		m_ptStart;
	CMenu		m_mPopupMenu;
	CDlgInput	m_dlgInput;
	CScene		m_scene;//ÃèÊöÕû¸ö³¡¾°

//	CFigure	m_Figure;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHumaView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CHumaView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CHumaView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRenderWireFrame();
	afx_msg void OnRenderHidenLine();
	afx_msg void OnRenderFlatShade();
	afx_msg void OnRenderFlatShadeLine();
	afx_msg void OnRenderSmoothShade();
	afx_msg void OnRenderFocus();
	afx_msg void OnRenderTexture();
	afx_msg void OnPickFace();
	afx_msg void OnPickEdge();
	afx_msg void OnPickVertex();
	afx_msg void OnMeterialBrass();
	afx_msg void OnMeterialBronze();
	afx_msg void OnMeterialChrome();
	afx_msg void OnMeterialCopper();
	afx_msg void OnMeterialGold();
	afx_msg void OnMeterialPolishedGold();
	afx_msg void OnMeterialPewter();
	afx_msg void OnMeterialSilver();
	afx_msg void OnMeterialPolishedSilver();
	afx_msg void OnMeterialEmerald();
	afx_msg void OnMeterialJade();
	afx_msg void OnMeterialObsidian();
	afx_msg void OnMeterialPearl();
	afx_msg void OnMeterialRudy();
	afx_msg void OnMeterialTurquoise();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in humaView.cpp
inline CHumaDoc* CHumaView::GetDocument()
   { return (CHumaDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HUMAVIEW_H__6E6339AE_343E_4E93_832B_BC5EFE7C97AD__INCLUDED_)
