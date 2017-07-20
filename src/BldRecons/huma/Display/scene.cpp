#include "scene.h"

CScene::CScene()
{
	m_MeshNum=0;
	m_pCurrentMesh=NULL;//当前选中的模型的Index
	m_RenderMode=RENDER_FLATSHADE_LINE;
	//视锥
	m_vEyePoint = Vector3D(0.0,0.0,3.0);
	m_vGazePoint = Vector3D(0.0,0.0,0.0);
	m_vUp = Vector3D(0.0,1.0,0.0);
	m_vFront = Vector3D(0.0,0.0,1.0);
	m_dDistance = 3.0;
//	m_nScale = 3.f/20.f;
	//光源
	m_vLight = Vector3D(-4.0,-4.0,-2.0);
	//拾取状态
	m_PickMode = PICK_NONE;

	m_Meterial = METERIAL_DEFAULT;
}

CScene::~CScene()
{
	
}

BOOL CScene::Create(HDC hDC)
{
	return m_Display.CreateOpenGL(hDC);
}
BOOL CScene::Create(HWND hWnd)
{
	HDC hDC = ::GetDC(hWnd);
	return m_Display.CreateOpenGL(hDC);


}
void CScene::DrawScene()
{
	if((m_MeshNum==0)||(m_RenderMode==RENDER_INVISIBLE))
		return;
//	m_vEyePoint=Vector3D(0.10059190653860,1.1857169213242,-3.2513044643689);
//	m_vGazePoint=Vector3D(-0.053492791690442,-0.11351067140378,-0.014990371303083);
//	m_vUp=Vector3D(-0.099069277851885,0.92506754418403,0.36665421978103);

	m_Display.LookAt(m_vEyePoint,m_vGazePoint,m_vUp);

//	m_Display.SetColor(1.f,1.f,1.f,true);//BackgroundColor

	m_Display.Lock();//所有的绘制操作必须放在Lock()和Unlock()之间

	m_Display.SetMeterial(m_Meterial);

	switch(m_PickMode)
	{
	case PICK_NONE:
		switch(m_RenderMode)
		{
		case RENDER_WIREFRAME:	//线框
			m_Display.RenderWireFrame();
			m_Display.GLColor3f(.7f,.7f,.7f);
			DrawMesh(m_pCurrentMesh,true);
			break;
		case RENDER_HIDENLINE:	//隐藏线
			m_Display.RenderHidenLine(true);
			DrawMesh(m_pCurrentMesh,true);
			m_Display.RenderHidenLine(false);
			m_Display.GLColor3f(.7f,.7f,.7f);
			DrawMesh(m_pCurrentMesh,true);
			break;
		case RENDER_FLATSHADE:		//flat shade
			m_Display.RenderFlatShade(false);
			DrawMesh(m_pCurrentMesh,true);
			break;
		case RENDER_FLATSHADE_LINE:	//flat shade with line
			m_Display.RenderFlatShade(true);
			DrawMesh(m_pCurrentMesh,true);
			m_Display.RenderHidenLine(false);
			m_Display.GLColor3f(0.1f,0.1f,0.1f);
			DrawMesh(m_pCurrentMesh,true);
			break;
		case RENDER_SMOOTHSHADE:	//smooth shade
			m_Display.RenderSmoothShade();
			DrawMesh(m_pCurrentMesh,false);
			break;
		case RENDER_FOCUS:			//flat shade with focus face, edge or vertex
			m_Display.RenderHidenLine(true);
			m_Display.GLColor3f(0.f,1.f,0.f);

			m_Display.RenderFlatShade(false);
			DrawMesh(m_pCurrentMesh,true);

			DrawPMesh(m_pCurrentMesh,true,false,false);
			m_Display.RenderHidenLine(false);
			DrawFocus(m_pCurrentMesh);
			break;
		case RENDER_TEXTURE:		//with texture
			DrawPMesh(m_pCurrentMesh,false,false,true);
			break;
		}
		break;
	case PICK_VERTEX:
		DrawVertex(m_pCurrentMesh,false);
		break;
	case PICK_FACE:
		DrawFace(m_pCurrentMesh,false);
		break;
	case PICK_EDGE:
		DrawEdge(m_pCurrentMesh,false);
		break;
	}

	m_Display.Unlock();
}

void CScene::DrawMesh(CMesh *pMesh, bool bFlat)
{
	for(UINT i = 0; i < pMesh->m_nFace; i++)
	{
		CFace* pFace=&(pMesh->m_pFace[i]);
		//temp for progressive
		if(pFace->m_bColapsed)
			continue;

		///////////////////////////////////////////////////////////////////////

		m_Display.GLBegin(DISPLAY_POLYGON);
		if(bFlat)
			m_Display.GLNormal(pFace->m_vNormal);
		for(short j = 0; j < pFace->m_nType; j++)
		{
			UINT l = pFace->m_piVertex[j];
			if(!bFlat)
				m_Display.GLNormal(pMesh->m_pVertex[l].m_vNormal);
			m_Display.GLVertex(pMesh->m_pVertex[l].m_vPosition);
		}
		m_Display.GLEnd();
	}

}

void CScene::DrawPMesh(CMesh *pMesh, bool bFlat, bool bColor, bool bTexture)
{
	
	if(bTexture)
	{
		glDisable(GL_LIGHTING);
		glEnable(GL_TEXTURE_2D);
		m_Display.GLColor3f(1.f,1.f,1.f);
		m_Display.GLBindTexture(0);
	}
/*
	glBegin(GL_QUADS);									// Start rendering QUADS		
		
		// Create a texture mapped QUAD that has the new texture map on it
		glTexCoord2f(   1, 0.0f);	glVertex3f(-1, -1,  0);
		glTexCoord2f(   1,    1);	glVertex3f(-1,  1,  0);
		glTexCoord2f(0.0f,    1);	glVertex3f( 1,  1,  0); 
		glTexCoord2f(0.0f, 0.0f);	glVertex3f( 1, -1,  0);

	glEnd();
*/

	for(UINT i = 0; i < pMesh->m_nFace; i++)
	{
		CFace* pFace=&(pMesh->m_pFace[i]);
		//temp for progressive
		if(pFace->m_bColapsed)
			continue;
		if((bColor)&&(!bTexture))
		{
			WORD c = pFace->m_color;
			BYTE r = c&0x000f;
			BYTE g = (c&0x00f0)>>4;
			BYTE b = (c&0x0f00)>>8;
			m_Display.GLColor3ub(r*16,g*16,b*16);
		}
		///////////////////////////////////////////////////////////////////////

		m_Display.GLBegin(DISPLAY_POLYGON);
		if(bFlat)
			m_Display.GLNormal(pFace->m_vNormal);
		for(short j = 0; j < pFace->m_nType; j++)
		{
			UINT l = pFace->m_piVertex[j];
			CEdge* pEdge = &(pMesh->m_pEdge[pFace->m_piEdge[j]]);
			CWedge* pWedge = &(pMesh->m_pWedge[pEdge->m_iWedge[0]]);
			if(bTexture)
				m_Display.GLTexture(pWedge->m_vParameter.x,pWedge->m_vParameter.y);
			if(!bFlat)
				m_Display.GLNormal(pMesh->m_pVertex[l].m_vNormal);
			m_Display.GLVertex(pMesh->m_pVertex[l].m_vPosition);
		}
		m_Display.GLEnd();
	}

	if(bTexture)
	{
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);
	}


}

void CScene::DrawFace(CMesh* pMesh, bool bPick)
{
	if (pMesh->m_nFace < 1)
		return;
	UINT i;
	m_Display.RenderHidenLine(true);
	DrawMesh(pMesh,true);
	m_Display.RenderHidenLine(false);
	m_Display.GLColor3f(0.f,1.f,0.f);
	DrawMesh(pMesh,true);
	

	int r,g,b;
	r=g=b=0;
	
	if(!bPick)
	{
		//绘制法向
		double dLen=0.01;//法向长度，可调节
		m_Display.GLBegin(DISPLAY_LINES);
		for(i = 0; i < pMesh->m_nFace; i++)
		{
			m_Display.GLVertex(pMesh->m_pFace[i].m_vMassPoint);
			m_Display.GLVertex(pMesh->m_pFace[i].m_vMassPoint+dLen*pMesh->m_pFace[i].m_vNormal);
		}
		m_Display.GLEnd();
		
	}
	
	m_Display.GLPointSize(4.0);
	m_Display.GLBegin(DISPLAY_POINTS);
	for(i = 0; i < pMesh->m_nFace; i++)
	{
		if(bPick)
		{
			b++;
			if(b>255)
			{
				b=0;
				g++;
			}
			if(g>255)
			{
				g=0;
				r++;
			}
			if(r>255)
				return;
			m_Display.GLColor3ub(r,g,b);//注意这里的rgb值从（0，0，1）开始
		}
		else
		{
			m_Display.GLColor3f(0.f,1.f,0.f);
			if(i==pMesh->m_iPickedFace)//绘制被选中的面
				m_Display.GLColor3f(1.f,0.f,0.f);
		}
		m_Display.GLVertex(pMesh->m_pFace[i].m_vMassPoint);
	}
	m_Display.GLEnd();
	m_Display.GLPointSize(1.0);

}

void CScene::DrawEdge(CMesh* pMesh, bool bPick)
{
	if (pMesh->m_nEdge < 1)
		return;
	m_Display.RenderHidenLine(true);
	DrawMesh(pMesh,true);

	int r,g,b;
	r=g=b=0;

	m_Display.GLBegin(DISPLAY_LINES);
	for(UINT i = 0; i < pMesh->m_nEdge; i++)
	{
		if(bPick)
		{
			b++;
			if(b>255)
			{
				b=0;
				g++;
			}
			if(g>255)
			{
				g=0;
				r++;
			}
			if(r>255)
				return;
			m_Display.GLColor3ub(r,g,b);//注意这里的rgb值从（0，0，1）开始
		}
		else
		{
			m_Display.GLColor3f(0.f,1.f,0.f);
			if(i==pMesh->m_iPickedEdge)//绘制被选中的边
				m_Display.GLColor3f(1.f,0.f,0.f);
		}
		m_Display.GLVertex(pMesh->m_pVertex[pMesh->m_pEdge[i].m_iVertex[0]].m_vPosition);
		m_Display.GLVertex(pMesh->m_pVertex[pMesh->m_pEdge[i].m_iVertex[1]].m_vPosition);
		
	}
	m_Display.GLEnd();

}

void CScene::DrawVertex(CMesh* pMesh, bool bPick)
{
	if (pMesh->m_nVertex < 1)
		return;
	m_Display.RenderHidenLine(true);
	DrawMesh(pMesh,true);

	int r,g,b;
	r=g=b=0;
	
	m_Display.GLPointSize(4.0);
	m_Display.GLBegin(DISPLAY_POINTS);
	for(UINT i = 0; i < pMesh->m_nVertex; i++)
	{
		if(bPick)
		{
			b++;
			if(b>255)
			{
				b=0;
				g++;
			}
			if(g>255)
			{
				g=0;
				r++;
			}
			if(r>255)
				return;
			m_Display.GLColor3ub(r,g,b);//注意这里的rgb值从（0，0，1）开始
		}
		else
		{
			m_Display.GLColor3f(0.f,1.f,0.f);
			if(i==pMesh->m_iPickedVertex)//绘制被选中的点
//			if((pMesh->m_pVertex[i].m_bIsBoundary)&&(pMesh->m_pVertex[i].m_nValence==1))
			m_Display.GLColor3f(1.f,0.f,0.f);
		}
		m_Display.GLVertex(pMesh->m_pVertex[i].m_vPosition);

	}
	m_Display.GLEnd();
	m_Display.GLPointSize(1.0);

	
}

void CScene::DrawFocus(CMesh* pMesh)
{
	UINT i;

	bool bDrawCutPath=false;
	bool bDrawEdge=true;
	bool bDrawFace=false;
	bool bDrawVertex=false;

	if(bDrawCutPath)
	{
		//绘制所有cut path上的边
		m_Display.GLLineWidth(6.0);
		m_Display.GLColor3f(1.f,0.f,0.f);
		m_Display.GLBegin(DISPLAY_LINES);
		for(i=0;i<pMesh->m_nEdge;i++)
		{
			CEdge* pEdge = &(pMesh->m_pEdge[i]);
			if(pEdge->m_bColapsed)
				continue;
			if(pEdge->m_bCut)
			{
				m_Display.GLVertex(pMesh->m_pVertex[pEdge->m_iVertex[0]].m_vPosition);
				m_Display.GLVertex(pMesh->m_pVertex[pEdge->m_iVertex[1]].m_vPosition);
			}
		}
		m_Display.GLEnd();
		
		//绘制所有边界上的点
		m_Display.GLPointSize(6.0);
		m_Display.GLBegin(DISPLAY_POINTS);
		for(i=0;i<pMesh->m_nVertex;i++)
		{
			CVertex* pVertex = &(pMesh->m_pVertex[i]);
			if(pVertex->m_bColapsed)
				continue;
			if(pVertex->m_nCut>0)
				m_Display.GLVertex(pMesh->m_pVertex[i].m_vPosition);
		}
		m_Display.GLEnd();
	}

	//绘制所有内部边
	m_Display.GLLineWidth(1.0);
	m_Display.GLColor3f(0.f,0.f,0.f);
	m_Display.GLBegin(DISPLAY_LINES);
	for(i=0;i<pMesh->m_nEdge;i++)
	{
		CEdge* pEdge = &(pMesh->m_pEdge[i]);
		if(pEdge->m_bColapsed)
			continue;
		if(!pEdge->m_bCut)
		{
			m_Display.GLVertex(pMesh->m_pVertex[pEdge->m_iVertex[0]].m_vPosition);
			m_Display.GLVertex(pMesh->m_pVertex[pEdge->m_iVertex[1]].m_vPosition);
		}
	}
	m_Display.GLEnd();

/*
	glDisable(GL_DEPTH_TEST);
	//绘制所有参数点
	m_Display.GLPointSize(3.0);
	m_Display.GLColor3f(1.f,1.f,0.f);
	m_Display.GLBegin(DISPLAY_POINTS);
	for(i=0;i<pMesh->m_nVertex;i++)
	{
		CVertex* pVertex = &(pMesh->m_pVertex[i]);
		if(!pVertex->m_bColapsed)
			m_Display.GLVertex(pMesh->m_pVertex[i].m_vParameter);
	}
	m_Display.GLEnd();
	m_Display.GLPointSize(1.0);

	//绘制所有参数域边界上的边
//	m_Display.GLLineWidth(2.0);
	m_Display.GLColor3f(0.f,0.f,0.f);
	m_Display.GLBegin(DISPLAY_LINES);
	for(i=0;i<pMesh->m_nEdge;i++)
	{
		CEdge* pEdge = &(pMesh->m_pEdge[i]);
		if(pEdge->m_bColapsed)
			continue;
		if(pEdge->m_bBoundary)
		{
			m_Display.GLVertex(pMesh->m_pVertex[pEdge->m_iVertex[0]].m_vParameter);
			m_Display.GLVertex(pMesh->m_pVertex[pEdge->m_iVertex[1]].m_vParameter);
		}
	}
	m_Display.GLEnd();

	//绘制所有参数域内部边
//	m_Display.GLLineWidth(1.0);
	m_Display.GLColor3f(0.f,0.f,0.f);
	m_Display.GLBegin(DISPLAY_LINES);
	for(i=0;i<pMesh->m_nEdge;i++)
	{
		CEdge* pEdge = &(pMesh->m_pEdge[i]);
		if(pEdge->m_bColapsed)
			continue;
		if(!pEdge->m_bBoundary)
		{
			m_Display.GLVertex(pMesh->m_pVertex[pEdge->m_iVertex[0]].m_vParameter);
			m_Display.GLVertex(pMesh->m_pVertex[pEdge->m_iVertex[1]].m_vParameter);
		}
	}
	m_Display.GLEnd();

	glEnable(GL_DEPTH_TEST);
*/	

	//draw Focus
	_UINTLIST::iterator it;
	
	if(bDrawEdge)
	{
		//绘制所有在pMesh->m_lFocusEdge表中的边
//		glDisable(GL_DEPTH_TEST);
		m_Display.GLLineWidth(5.0);
		m_Display.GLColor3f(0.f,0.f,0.f);
		it = pMesh->m_lFocusEdge.begin();
		m_Display.GLBegin(DISPLAY_LINES);
		for(i=0;i<pMesh->m_lFocusEdge.size();i++)
		{
			CEdge* pEdge = &(pMesh->m_pEdge[*(it++)]);
			m_Display.GLVertex(pMesh->m_pVertex[pEdge->m_iVertex[0]].m_vPosition);
			m_Display.GLVertex(pMesh->m_pVertex[pEdge->m_iVertex[1]].m_vPosition);

		}
		m_Display.GLEnd();
//		glEnable(GL_DEPTH_TEST);

	}

	if(bDrawFace)
	{
		//绘制所有在pMesh->m_lFocusFace表中的面
		m_Display.GLPointSize(6.0);
		m_Display.GLColor3f(1.f,0.f,0.f);
		it = pMesh->m_lFocusFace.begin();
		m_Display.GLBegin(DISPLAY_POINTS);
		for(i=0;i<pMesh->m_lFocusFace.size();i++)
		{
			CFace* pFace=&(pMesh->m_pFace[*it++]);
			m_Display.GLVertex(pFace->m_vMassPoint);
		}
		m_Display.GLEnd();
	}

	if(bDrawVertex)
	{
		//绘制所有在pMesh->m_lFocusVertex表中的点
		m_Display.GLPointSize(6.0);
		m_Display.GLColor3f(1.f,0.f,0.f);
		it = pMesh->m_lFocusVertex.begin();
		m_Display.GLBegin(DISPLAY_POINTS);
		for(i=0;i<pMesh->m_lFocusVertex.size();i++)
		{
			CVertex* pVertex=&(pMesh->m_pVertex[*it++]);
			m_Display.GLVertex(pVertex->m_vPosition);
		}
		m_Display.GLEnd();
	}
	
	m_Display.GLLineWidth(1.0);
	m_Display.GLPointSize(1.0);

}

BOOL CScene::AddMesh(CMesh *pNewMesh)
{
	if(m_MeshNum==MAX_MODEL_NUM)
		return FALSE;
	m_MeshPointers[m_MeshNum]=pNewMesh;
	m_pCurrentMesh=pNewMesh;
	m_MeshNum++;

	return TRUE;
}


void CScene::Move(int x,int y)
{
	Vector3D w=m_vUp^m_vFront;
	Vector3D W=w*(float)m_dDistance*(float)x/(float)m_nWidth;
	Vector3D U=m_vUp*(float)m_dDistance*(float)y/(float)m_nWidth;
	m_vEyePoint+=W+U;
	m_vGazePoint+=W+U;
}

void CScene::Rotate(double a,double b)
{
	Vector3D w=m_vUp^m_vFront;
	w.normalize();
	m_vFront=m_vFront*(float)cos(a)+w*(float)sin(a);

	w=m_vUp^m_vFront;
	w.normalize();
	m_vFront=m_vFront*(float)cos(b)+m_vUp*(float)sin(b);
	m_vUp=m_vFront^w;
	
	m_vEyePoint=m_vGazePoint+m_vFront*(float)m_dDistance;

}

void CScene::Scale(int d)
{
	m_dDistance*=1.0+(double)d/(double)m_nWidth;
	m_vEyePoint=m_vGazePoint+m_vFront*(float)m_dDistance;
}

void CScene::MouseMove(UINT nFlags, int cx, int cy)
{
	if(nFlags & MK_SHIFT)
		Scale(cx);
	else if(nFlags & MK_CONTROL)
		Move(cx,-cy);
	else if(::GetKeyState(VK_MENU) & 0xf0)
		Rotate(cx/300.f,-cy/300.0);
//	m_nScale=m_vEyePoint.length()/20.0;
}

void CScene::Reshape(int cx, int cy) 
{
	if( cx>0 && cy>0 )
	{
		m_nWidth=cx;
		m_nHeight=cy;
		m_Display.Reshape(cx,cy);
	}
}

void CScene::SetRenderMode(BYTE mode)
{
	m_RenderMode=mode;
}

void CScene::SetPickMode(BYTE mode)
{
	m_PickMode=mode;
}

void CScene::SetMeterial(BYTE meterial)
{
//	m_Display.SetMeterial(meterial);
	m_Meterial = meterial;
}
void CScene::Pick(int x, int y)
{
	if(m_PickMode==PICK_NONE)
		return;

	m_Display.BeginPick();//将要拾取的对象用不同的RGB值绘制到BeginPick()和EndPick()之间

	switch(m_PickMode)
	{
	case PICK_FACE:
		DrawFace(m_pCurrentMesh,true);
		break;
	case PICK_EDGE:
		DrawEdge(m_pCurrentMesh,true);
		break;
	case PICK_VERTEX:
		DrawVertex(m_pCurrentMesh,true);
	}
	
	int n=m_Display.EndPick(x,y);//EndPick()返回ture后，即可从m_glcanvas的m_Picking中得到所有拾取区域内的象素的RGB值

	switch(m_PickMode)
	{
	case PICK_FACE:
		PickFace(m_pCurrentMesh,n,m_Display.m_PickNum);
		break;
	case PICK_EDGE:
		PickEdge(m_pCurrentMesh,n,m_Display.m_PickNum);
		break;
	case PICK_VERTEX:
		PickVertex(m_pCurrentMesh,n,m_Display.m_PickNum);
	}
}

void CScene::PickFace(CMesh *pMesh, int n,UINT *pLong)
{
	if(n==0)
		pMesh->m_iPickedFace=-1;
	else
		pMesh->m_iPickedFace=pLong[0]-1;
}

void CScene::PickEdge(CMesh *pMesh, int n,UINT *pLong)
{
	if(n==0)
		pMesh->m_iPickedEdge=-1;
	else
		pMesh->m_iPickedEdge=pLong[0]-1;
}

void CScene::PickVertex(CMesh *pMesh, int n,UINT *pLong)
{
	if(n==0)
		pMesh->m_iPickedVertex=-1;
	else
		pMesh->m_iPickedVertex=pLong[0]-1;
}