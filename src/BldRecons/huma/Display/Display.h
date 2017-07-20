// Display.h: interface for the Display class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DISPLAY_H__34F69226_DB0E_4C42_A179_0C053E30F918__INCLUDED_)
#define AFX_DISPLAY_H__34F69226_DB0E_4C42_A179_0C053E30F918__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "gl/gl.h"
#include "gl/glu.h"
//#include "../gl/glaux.h"
#include "..\common\geometry.h"

#define DISPLAY_INTERFACE_OPENGL	1
#define DISPLAY_INTERFACE_DIRECT3D  2

//绘制对象
#define DISPLAY_POINTS		1
#define DISPLAY_LINES		2
#define DISPLAY_TRIANGLES	3
#define DISPLAY_QUADS		4
#define DISPLAY_POLYGON		5

#define PICKRANGE 4             //拾取的范围
#define HALFPICKRANGE 2

//材质
#define METERIAL_DEFAULT	0
#define METERIAL_BRASS		1
#define METERIAL_BRONZE		2
#define METERIAL_CHROME		3
#define METERIAL_COPPER		4
#define METERIAL_GOLD		5
#define METERIAL_P_GOLD		6
#define METERIAL_PEWTER		7
#define METERIAL_SILVER		8
#define METERIAL_P_SILVER	9
#define METERIAL_EMERALD	10
#define METERIAL_JADE		11
#define METERIAL_OBSIDIAN	12
#define METERIAL_PEARL		13
#define METERIAL_RUDY		14
#define METERIAL_TURQUOISE	15


class CDisplay  
{
public:
	CDisplay();
	virtual ~CDisplay();
	BOOL	CreateOpenGL(HDC hDC);
	void	LookAt(Vector3D vEye,Vector3D vGaze,Vector3D vUp);
	void	Reshape(int cx,int cy);
	void	Lock();
	void	Unlock();
	void	GLBegin(BYTE mode);
	void	GLEnd();
	void	GLVertex(Vector3D v);
	void	GLNormal(Vector3D v);
	void	GLTexture(double s,double t);
	void	GLPointSize(float f);
	void	GLLineWidth(float f);
	void	GLColor3f(float r, float g, float b);
	void	GLColor3ub(BYTE r, BYTE g, BYTE b);
	void	SetBGColor(float r, float g, float b);
	void	SetMeterial(BYTE meterial);
	void	BeginPick();
	int		EndPick(int x,int y);
	void	GLBindTexture(int iTexture);

	//设置场景的渲染方式
	void	RenderWireFrame();
	void	RenderHidenLine(bool bBack);
	void	RenderFlatShade(bool bLine);
	void	RenderSmoothShade();

private:
	BOOL	SetWindowPixelFormat(HDC hDC);
	BOOL	CreateViewGLContext(HDC hDC);
	void	buildList();
//	void	createTexture(UINT textureArray[], LPSTR strFileName, int textureID);

public:
	BYTE	m_Interface;//OpenGL or Direct3D
	HDC		m_hDC;
	Vector3D m_vEyePoint;
	Vector3D m_vGazePoint;
	Vector3D m_vUp;
	float	m_BGcolor[3];
	int		m_Width;
	int		m_Height;

	//for OpenGL
	int		m_GLPixelIndex;
	HGLRC	m_hGLContext;
	GLuint	m_ListNet;
	GLuint	m_ListXYZ;

	//picking
	UINT m_PickNum[PICKRANGE*PICKRANGE];//将RGB值转成长整型值
	GLubyte m_Pick[PICKRANGE][PICKRANGE][3];//存储选中区域内所有点的RGB值

	UINT m_Texture[10];

};

#endif // !defined(AFX_DISPLAY_H__34F69226_DB0E_4C42_A179_0C053E30F918__INCLUDED_)
