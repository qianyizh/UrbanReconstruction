///////////////////////class CMesh////////////////////////////////////
//		可以处理任意拓扑，任意多边形的二流型网格（2-manifode mesh)	//
//		可以读取一般的obj,smf,wrl等文件类型							//
//		根据点表和面表生成完整的连接关系							//
//																	//
//////////////////////////////////////////////////////////////////////
#ifndef _MESH_H_
#define _MESH_H_

#include "model.h"
#include <afx.h>
#include <list>

using namespace std;
typedef list<Vector3D>	_VECTORLIST;
typedef list<UINT>		_UINTLIST;

#define MAX_VERTEX_PER_FACE 20

class CVertex
{
public:
	Vector3D	m_vPosition;//点的坐标
	UINT*		m_piEdge;//从该点发出的halfedge,要根据点的度数动态创建
	_UINTLIST	m_lEdgeList;//用来构造m_piEdge的临时链表
	short		m_nValence;//点的度数
	bool		m_bIsBoundary;//是否为边界点
	Vector3D	m_vNormal;
	Vector3D	m_vMeanCurvature;//Mean curvature

	bool		m_bColapsed;//是否已经被删掉
	short		m_nCut;//该点与几条cut path相连
//	Vector3D	m_vParameter;//该点的参数

public:
	//constructions
	CVertex() { m_piEdge=NULL; m_nValence=0; m_bIsBoundary=false;m_bColapsed=false;m_nCut=0;}
	CVertex(double x,double y,double z) {m_vPosition=Vector3D(x,y,z);m_piEdge=NULL; m_nValence=0;m_bIsBoundary=false;m_bColapsed=false;m_nCut=0;}
	CVertex(Vector3D v) {m_vPosition=v;m_piEdge=NULL; m_nValence=0;m_bIsBoundary=false;m_bColapsed=false;m_nCut=0;}
	virtual ~CVertex();

	//operations
	CVertex& operator = (CVertex& v);

};

class CWedge
{
public:
	Vector2D	m_vParameter;
	UINT	m_iVertex;
public:
	CWedge()	{m_iVertex=-1;}
	CWedge(UINT iVertex)	{m_iVertex=iVertex;}
	virtual ~CWedge();
};

class CEdge
{
public:
	UINT	m_iVertex[2];//边的两端点，Vertex0－>Vertex1
	UINT	m_iWedge[2];//该边指向的两个Wedge,其实有这个就可以不用m_iVertex[2]了

	UINT	m_iTwinEdge;//与该边方向相反的另一条边，如果为-1则该边为边界
	UINT	m_iNextEdge;//沿逆时针方向的下一条边
//	UINT	m_iPreEdge;
	UINT	m_iFace;//该边所属的面，应该在它的左边

	//for progressive mesh
	double	m_dPriority;//该边化简时的优先值
//	double	m_dQEM;//该边化简时的优先值
	bool	m_bColapsed;//是否已经被删掉
	bool	m_bLegal;//删除该边是否合法，3条准则
	bool	m_bCut;//是否为剪裁边
	UINT	m_iIndex;//在EdgeArray中的Index值



public:
	//constructions
	CEdge() {m_iVertex[0]=m_iVertex[1]=m_iWedge[0]=m_iWedge[1]=m_iTwinEdge=m_iNextEdge=m_iFace=-1;m_bColapsed=false;m_bLegal=true;m_bCut=false;}
	CEdge(UINT iV0, UINT iV1) { m_iVertex[0]=iV0; m_iVertex[1]=iV1; m_iWedge[0]=m_iWedge[1]=-1;m_iTwinEdge=m_iNextEdge=m_iFace=-1;m_bColapsed=false;m_bLegal=true;m_bCut=false;}
	virtual ~CEdge();

	//operations
	CEdge& operator = (const CEdge& e);
	
	
};

class CFace
{
public:
	short	m_nType;//几边形
	UINT*	m_piVertex;//所有点
	UINT*	m_piEdge;//所有边
	Vector3D m_vNormal;//法向
	Vector3D m_vMassPoint;//法向
	double	m_dArea;//面积

	//for progressive mesh
	bool	m_bColapsed;
	WORD	m_color;//面的颜色
	double	m_dSumArea;//压缩过程中包含的所有面的面积和
	double	m_dStretch;//Geometry Stretch

public:
	//constructions
	CFace() {m_nType=0;m_piVertex=m_piEdge=NULL;m_vNormal=Vector3D(0.0,0.0,1.0);m_dArea=0.0;m_bColapsed=false;m_color=0;}
	CFace(short s);
	virtual ~CFace();

	//operations
	void Create(short s);
	CFace& operator = (const CFace& f);


};

class CEdgeArray//for progressmesh
{
public:
	CEdge*	m_pEdge;
	UINT*	m_aEdge;//array
	UINT	m_nValidEdge;//left edge number
	UINT	m_nTotalEdge;
public:
	CEdgeArray() {m_pEdge=NULL;m_aEdge=NULL;m_nValidEdge=0;}
	virtual ~CEdgeArray();
	void Create(CEdge* pEdge,UINT nValidEdge);
	UINT GetNextEdge();
	void RemoveEdge(UINT iEdge);
	void RestoreEdge(UINT iEdge);

	CEdgeArray& operator = (CEdgeArray& ea);

};


class CMesh :public CModel
{
public:
	UINT		m_nVertex;				//点数
	CVertex*	m_pVertex;				//点表
	UINT		m_nEdge;				//边数
	CEdge*		m_pEdge; 				//边表
	UINT		m_nFace;	 			//面数
	CFace*		m_pFace;				//面表
	UINT		m_nWedge;				//Wedge数
	CWedge*		m_pWedge;				//Wedge表

	//temp
//	_UINTLIST m_lPickedFace;
	_UINTLIST m_lFocusEdge;
	_UINTLIST m_lFocusVertex;
	_UINTLIST m_lFocusFace;
//	_UINTLIST m_lPickedVertex;
	UINT	m_iPickedFace;
	UINT	m_iPickedEdge;
	UINT	m_iPickedVertex;

	bool	m_bClosed;

public:
	//constructions
	CMesh() {m_nVertex=m_nEdge=m_nWedge=m_nFace=0;m_pVertex=NULL;m_pEdge=NULL;m_pWedge=NULL;m_pFace=NULL;m_iPickedFace=m_iPickedEdge=m_iPickedVertex=-1;}
	CMesh(CMesh* pMesh);
	virtual ~CMesh();

	//operations
	//输入输出
public:
	BOOL	Load(CString sFileName);	// load from file
	BOOL	Save(CString sFileName);	// save to file
	double	GetEdgeLen(UINT iEdge);
	double  GetEdgeWeight(UINT iEdge);

	BOOL	construct();// construct connectivity

private:
	void	clear();
//	BOOL	construct();// construct connectivity
	BOOL	reConstruct();// construct connectivity from current mesh
	BOOL	loadFromSMF(CString sFileName);
	BOOL	loadFromWRL(CString sFileName);
	BOOL	loadFromTMP(CString sFileName);
	BOOL	loadFromGTS(CString sFileName);
	BOOL	saveToSMF(CString sFileName);
	void	calFaceNormal(UINT i);
	void	calVertexNormal(UINT i);
	void	calVertexMeanCurvature(UINT i);

};

#endif