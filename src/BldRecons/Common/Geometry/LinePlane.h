#pragma once

#include "Vector3D.h"

class CLine
{
public:
	CVector3D p;				// p[2] == 0
	CVector3D d;				// d[2] == 0, |d| == 1

	double dis( CVector3D & v ) {
		CVector3D n( d[ 1 ], -d[ 0 ], 0.0 );
		return abs( ( v - p ) * n );
	}

	CVector3D n() {
		return CVector3D( d[ 1 ], -d[ 0 ], 0.0 );
	}

	CVector3D project( CVector3D & v ) {
		CVector3D n( d[ 1 ], -d[ 0 ], 0.0 );
		return ( v - ( ( v - p ) * n ) * n );
	}

	bool check( CVector3D & v, double tolerance ) {
		return dis( v ) < tolerance;
	}

	friend CVector3D operator^( CLine & line0, CLine & line1 ) {
		CVector3D vtemp;

		double c0 = line0.d[0];
		double s0 = line0.d[1];
		double c1 = line1.d[0];
		double s1 = line1.d[1];

		if ( abs( s0 * c1 - s1 * c0 ) < DOUBLE_TOLERANCE ) {
			return CVector3D::INFINITE_VECTOR3D;
		}

		CVector3D & v0 = line0.p;
		CVector3D & v1 = line1.p;

		vtemp[0] = ( c0 * c1 * ( v1[1] - v0[1] ) + s0 * c1 * v0[0] - s1 * c0 * v1[0] ) / ( s0 * c1 - s1 * c0 );
		vtemp[1] = ( s0 * s1 * ( v1[0] - v0[0] ) + c0 * s1 * v0[1] - c1 * s0 * v1[1] ) / ( c0 * s1 - c1 * s0 );
		vtemp[2] = 0.0;

		return vtemp;
	}
};

class CPlane
{
public:
	CVector3D pos;
	CVector3D norm;				// |norm| = 1
	int index;

	double dis( CVector3D & v ) {
		return ( v - pos ) * norm;
	}

	CVector3D project( CVector3D & v ) {
		return ( v - dis( v ) * norm );
	}

	CVector3D onplane( CVector3D & v ) {
		if ( abs( norm[ 2 ] ) < DOUBLE_TOLERANCE )
			return CVector3D::INFINITE_VECTOR3D;
		else
			return CVector3D( v[0], v[1], pos[2] - ( norm[0] * ( v[0] - pos[0] ) + norm[1] * ( v[1] - pos[1] ) ) / norm[2] );
	}
};
