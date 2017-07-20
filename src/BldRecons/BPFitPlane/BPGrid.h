#pragma once

#include "BPCloud.h"
#include "Geometry\Outline.h"

typedef std::vector< int > PatchVector;

class CBPGrid
{
public:
	CBPGrid( CBPCloud * pCloud );
	~CBPGrid(void);

public:
	std::vector< PatchPointDataVector > m_vecGridIndex;
	std::vector< PatchVector > m_vecGridPatch;
	std::vector< CPlane > m_vecPlane;
	std::vector< COutline > m_vecOutline;

protected:
	CBPCloud * m_pCloud;

public:
	void BuildGridIndex();
	void PlaneFitting_RegionGrow();
	void GenerateOutline();
	void WriteOutline( char filename[] );

protected:
	int Index( const CVector3D & v );
	int Index( int x, int y );
	int IndexX( const CVector3D & v );
	int IndexY( const CVector3D & v );
	void RegularizeX( int & x );
	void RegularizeY( int & y );

protected:
	CPlane FitPlane( PatchPointDataVector & data );
	void RollBack( PatchPointDataVector & data );
	void PlaneFitting_RegionGrow_NormalCheck( CPlane & plane, PatchPointDataVector & data_nc, PatchPointData & seed );
	void PlaneFitting_RegionGrow_DoubleCheck( CPlane & plane, PatchPointDataVector & data_nc, PatchPointDataVector & data_dc, PatchPointData & seed );

protected:
	bool CheckPatchVector( int index, int patch );
	void PushPatchVector( int index, int patch );

protected:
	void InitOutline();
	void GenerateSingleOutline( int plane_index );
};
