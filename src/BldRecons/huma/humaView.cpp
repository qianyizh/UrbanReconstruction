// humaView.cpp : implementation of the CHumaView class
//

#include "stdafx.h"
#include "huma.h"

#include "humaDoc.h"
#include "humaView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CHumaView

IMPLEMENT_DYNCREATE(CHumaView, CView)

BEGIN_MESSAGE_MAP(CHumaView, CView)
	//{{AFX_MSG_MAP(CHumaView)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(ID_RENDER_WIREFRAME, OnRenderWireFrame)
	ON_COMMAND(ID_RENDER_HIDENLINE, OnRenderHidenLine)
	ON_COMMAND(ID_RENDER_FLATSHADE, OnRenderFlatShade)
	ON_COMMAND(ID_RENDER_FLATSHADE_LINE, OnRenderFlatShadeLine)
	ON_COMMAND(ID_RENDER_SMOOTHSHADE, OnRenderSmoothShade)
	ON_COMMAND(ID_RENDER_FOCUS, OnRenderFocus)
	ON_COMMAND(ID_RENDER_FOCUS, OnRenderTexture)
	ON_COMMAND(ID_PICK_FACE, OnPickFace)
	ON_COMMAND(ID_PICK_EDGE, OnPickEdge)
	ON_COMMAND(ID_PICK_VERTEX, OnPickVertex)
	ON_COMMAND(ID_METERIAL_BRASS, OnMeterialBrass)
	ON_COMMAND(ID_METERIAL_BRONZE, OnMeterialBronze)
	ON_COMMAND(ID_METERIAL_CHROME, OnMeterialChrome)
	ON_COMMAND(ID_METERIAL_COPPER, OnMeterialCopper)
	ON_COMMAND(ID_METERIAL_GOLD, OnMeterialGold)
	ON_COMMAND(ID_METERIAL_P_GOLD, OnMeterialPolishedGold)
	ON_COMMAND(ID_METERIAL_PEWTER, OnMeterialPewter)
	ON_COMMAND(ID_METERIAL_SILVER, OnMeterialSilver)
	ON_COMMAND(ID_METERIAL_P_SILVER, OnMeterialPolishedSilver)
	ON_COMMAND(ID_METERIAL_EMERALD, OnMeterialEmerald)
	ON_COMMAND(ID_METERIAL_JADE, OnMeterialJade)
	ON_COMMAND(ID_METERIAL_OBSIDIAN, OnMeterialObsidian)
	ON_COMMAND(ID_METERIAL_PEARL, OnMeterialPearl)
	ON_COMMAND(ID_METERIAL_RUDY, OnMeterialRudy)
	ON_COMMAND(ID_METERIAL_TURQUOISE, OnMeterialTurquoise)
	ON_WM_LBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHumaView construction/destruction

CHumaView::CHumaView()
{
	// TODO: add construction code here
}

CHumaView::~CHumaView()
{
}

BOOL CHumaView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CHumaView drawing

void CHumaView::OnDraw(CDC* pDC)
{
	// TODO: add draw code for native data here
}

/////////////////////////////////////////////////////////////////////////////
// CHumaView diagnostics

#ifdef _DEBUG
void CHumaView::AssertValid() const
{
	CView::AssertValid();
}

void CHumaView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CHumaDoc* CHumaView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CHumaDoc)));
	return (CHumaDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CHumaView message handlers

int CHumaView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	// TODO: Add your specialized creation code here
//	CRect rect(100,100,400,400);
//	m_image.Create(rect,this);
	InitUI();
	HWND hWnd = GetSafeHwnd();
	return m_scene.Create(hWnd);
}

void CHumaView::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	m_scene.DrawScene();

}

void CHumaView::OnMouseMove(UINT nFlags, CPoint point) 
{
if (nFlags & MK_LBUTTON)
	{
		CSize dif = m_ptStart - point;
		m_ptStart = point;
		m_scene.MouseMove(nFlags,dif.cx,dif.cy);
		Invalidate(FALSE);
	}	
	CView::OnMouseMove(nFlags, point);
}


BOOL CHumaView::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default
	
	return TRUE;
}

void CHumaView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	m_scene.Reshape(cx,cy);
	Invalidate(FALSE);	
}

void CHumaView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	// popup pos
	ClientToScreen(&point);
	// Popup the menu
	CMenu *pmenu = m_mPopupMenu.GetSubMenu(0);
	pmenu->TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this);	
	CView::OnRButtonDown(nFlags, point);
}

void CHumaView::InitUI()
{
	m_mPopupMenu.LoadMenu(IDR_POPUP);
}

void CHumaView::OnRenderWireFrame()
{
	m_scene.SetRenderMode(RENDER_WIREFRAME);
	m_scene.SetPickMode(PICK_NONE);
	Invalidate(FALSE);
}
void CHumaView::OnRenderHidenLine()
{
	m_scene.SetRenderMode(RENDER_HIDENLINE);
	m_scene.SetPickMode(PICK_NONE);
	Invalidate(FALSE);
}
void CHumaView::OnRenderFlatShade()
{
	m_scene.SetRenderMode(RENDER_FLATSHADE);
	m_scene.SetPickMode(PICK_NONE);
	Invalidate(FALSE);
}
void CHumaView::OnRenderFlatShadeLine()
{
	m_scene.SetRenderMode(RENDER_FLATSHADE_LINE);
	m_scene.SetPickMode(PICK_NONE);
	Invalidate(FALSE);
}
void CHumaView::OnRenderSmoothShade()
{
	m_scene.SetRenderMode(RENDER_SMOOTHSHADE);
	m_scene.SetPickMode(PICK_NONE);
	Invalidate(FALSE);
}

void CHumaView::OnRenderFocus()
{
	m_scene.SetRenderMode(RENDER_FOCUS);
	m_scene.SetPickMode(PICK_NONE);
	Invalidate(FALSE);
}

void CHumaView::OnRenderTexture()
{
	m_scene.SetRenderMode(RENDER_TEXTURE);
	m_scene.SetPickMode(PICK_NONE);
	Invalidate(FALSE);
}

void CHumaView::OnMeterialBrass()
{
	m_scene.SetMeterial(METERIAL_BRASS);
	Invalidate(FALSE);
}

void CHumaView::OnMeterialBronze()
{
	m_scene.SetMeterial(METERIAL_BRONZE);
	Invalidate(FALSE);
}

void CHumaView::OnMeterialChrome()
{
	m_scene.SetMeterial(METERIAL_CHROME);
	Invalidate(FALSE);
}

void CHumaView::OnMeterialCopper()
{
	m_scene.SetMeterial(METERIAL_COPPER);
	Invalidate(FALSE);
}

void CHumaView::OnMeterialGold()
{
	m_scene.SetMeterial(METERIAL_GOLD);
	Invalidate(FALSE);
}

void CHumaView::OnMeterialPolishedGold()
{
	m_scene.SetMeterial(METERIAL_P_GOLD);
	Invalidate(FALSE);
}

void CHumaView::OnMeterialPewter()
{
	m_scene.SetMeterial(METERIAL_PEWTER);
	Invalidate(FALSE);
}

void CHumaView::OnMeterialSilver()
{
	m_scene.SetMeterial(METERIAL_SILVER);
	Invalidate(FALSE);
}
void CHumaView::OnMeterialPolishedSilver()
{
	m_scene.SetMeterial(METERIAL_P_SILVER);
	Invalidate(FALSE);
}
void CHumaView::OnMeterialEmerald()
{
	m_scene.SetMeterial(METERIAL_EMERALD);
	Invalidate(FALSE);
}
void CHumaView::OnMeterialJade()
{
	m_scene.SetMeterial(METERIAL_JADE);
	Invalidate(FALSE);
}
void CHumaView::OnMeterialObsidian()
{
	m_scene.SetMeterial(METERIAL_OBSIDIAN);
	Invalidate(FALSE);
}
void CHumaView::OnMeterialPearl()
{
	m_scene.SetMeterial(METERIAL_PEARL);
	Invalidate(FALSE);
}
void CHumaView::OnMeterialRudy()
{
	m_scene.SetMeterial(METERIAL_RUDY);
	Invalidate(FALSE);
}
void CHumaView::OnMeterialTurquoise()
{
	m_scene.SetMeterial(METERIAL_TURQUOISE);
	Invalidate(FALSE);
}

void CHumaView::OnPickFace()
{
	m_scene.SetPickMode(PICK_FACE);
	Invalidate(FALSE);
}

void CHumaView::OnPickEdge()
{
	m_scene.SetPickMode(PICK_EDGE);
	Invalidate(FALSE);
}

void CHumaView::OnPickVertex()
{
	m_scene.SetPickMode(PICK_VERTEX);
	Invalidate(FALSE);
}

void CHumaView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	m_ptStart = point;
	CView::OnLButtonDown(nFlags, point);
}

void CHumaView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default

	if(nFlags & MK_CONTROL)
		return;
	if(nFlags & MK_SHIFT)
		return;
	if(::GetKeyState(VK_MENU) & 0xf0)
		return;

	m_scene.Pick(point.x, point.y);
	Invalidate(FALSE);

	CView::OnLButtonUp(nFlags, point);
}

