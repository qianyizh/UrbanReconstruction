#pragma once

#include "Geometry\Vector3D.h"
#include <vector>

enum PointType {
	PT_Unclassified = 0,
	PT_Ground = 1,
	PT_Building = 2,
	PT_Tree = 3,
	PT_Noise = 4
};

struct PointData {
	PointType type;
	CVector3D v;
	CVector3D n;
	double feature[ 5 ];
	double buildingness;
};

typedef std::vector< PointData * > PointDataVector;

typedef __int64 PatchIndex;

struct PatchInfo {
	PatchIndex base;
	int num;
};

struct PatchInfoEx : public PatchInfo {
	double height;
};

struct PatchPointData {
	PointType type;
	PatchInfo patch;
	CVector3D v;
	CVector3D n;
	double flatness;
};

typedef std::vector< PatchPointData * > PatchPointDataVector;

#define GLOBAL_INDEX( i ) ( int )( ( ( i ) >> 32 ) & 0xffffffff )
#define LOCAL_INDEX( i ) ( int )( ( i ) & 0xffffffff )
#define PATCH_INDEX( i, j ) ( ( ( __int64 )( i ) ) << 32 ) | ( __int64 )( j )