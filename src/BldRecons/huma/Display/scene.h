///////////////////////class CScene///////////////////////////////////
//						场景的组织，绘制，交互						//
//////////////////////////////////////////////////////////////////////
#ifndef _SCENE_H_
#define _SCENE_H_

#include "..\model\mesh.h"
#include "display.h"

#define MAX_MODEL_NUM 0xff

//场景的渲染方式
#define RENDER_INVISIBLE		0	//不可见
#define RENDER_WIREFRAME		1	//线框
#define RENDER_HIDENLINE		2	//隐藏线
#define RENDER_FLATSHADE		3	//flat shade
#define RENDER_FLATSHADE_LINE	4	//flat shade with line
#define RENDER_SMOOTHSHADE		5	//smooth shade
#define RENDER_FOCUS			6	//focus edge
#define RENDER_TEXTURE			7	//with texture

//拾取状态
#define PICK_NONE	0
#define PICK_VERTEX	1
#define PICK_EDGE	2
#define PICK_FACE	3


class CScene
{
public:
	CScene();
	virtual ~CScene();
	BOOL	Create(HDC hDC);
	BOOL	Create(HWND hWnd);
	void	DrawScene();
	BOOL	AddMesh(CMesh* pNewMesh);
	
	//视角变换
	void	Move(int x,int y);
	void	Rotate(double a,double b);
	void	Scale(int d);
	void	MouseMove(UINT nFlags, int cx, int cy);//鼠标移动时变换视角
	void	Reshape(int cx,int cy);//改变viewport的大小

	//改变场景的渲染方式,整个场景用一样的渲染方式
	void	SetRenderMode(BYTE mode);

	//改变材质
	void	SetMeterial(BYTE meterial);

	//拾取
	void	SetPickMode(BYTE mode);//设置拾取对象
	void	Pick(int x, int y);//完成拾取操作

private:
	void	DrawMesh(CMesh *pMesh, bool bFlat);
	void	DrawPMesh(CMesh *pMesh, bool bFlat, bool bColor, bool bTexture);//Progressive Mesh
	void	DrawFace(CMesh *pMesh, bool bPick);
	void	DrawEdge(CMesh *pMesh, bool bPick);
	void	DrawVertex(CMesh *pMesh, bool bPick);
	void	DrawFocus(CMesh *pMesh);

	void	PickFace(CMesh *pMesh, int n,UINT *pLong);
	void	PickEdge(CMesh *pMesh, int n,UINT *pLong);
	void	PickVertex(CMesh *pMesh, int n,UINT *pLong);

	void CreateTexture(UINT textureArray[], LPSTR strFileName, int textureID);

public:
	CDisplay m_Display;//显示接口,可以选择用什么方式显示,OPENGL or DIRECT3D
	CMesh*	m_MeshPointers[MAX_MODEL_NUM];//所有模型的指针
	

private:
	BYTE	m_MeshNum;//场景中模型的个数
	BYTE	m_RenderMode;//场景的渲染方式,整个场景用一样的渲染方式
	BYTE	m_Meterial;
	CMesh*	m_pCurrentMesh;//当前选中的模型

	//视锥
	Vector3D m_vEyePoint;
	Vector3D m_vGazePoint;
	Vector3D m_vUp;
	Vector3D m_vFront;
	double	m_dDistance;

	//viewport
	int		m_nWidth;
	int		m_nHeight;

	//光源，目前规定只有一个光源
	Vector3D m_vLight;

	//拾取状态
	BYTE	m_PickMode;//拾取对象

};

#endif