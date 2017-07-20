#pragma once

#include <math.h>

#define DOUBLE_TOLERANCE 1e-10

class CVector3D
{
public:
	CVector3D(void) { pVec[0] = pVec[1] = pVec[2] = 0.0; }
	CVector3D(double x, double y, double z) { pVec[0] = x; pVec[1] = y; pVec[2] = z; }
	CVector3D(double xx[3]) { pVec[0] = xx[0]; pVec[1] = xx[1], pVec[2] = xx[2]; }
	CVector3D(const CVector3D & v) { pVec[0] = v.pVec[0]; pVec[1] = v.pVec[1]; pVec[2] = v.pVec[2]; }
	~CVector3D(void) {}

public :
	double pVec[3];

	// operator
	double	  length()		{	return sqrt(pVec[0] * pVec[0] + pVec[1] * pVec[1] + pVec[2] * pVec[2]);	}
	double	  length2()		{	return pVec[0] * pVec[0] + pVec[1] * pVec[1] + pVec[2] * pVec[2];	}
	double	  normalize()	{	double len = length();	if (abs(len) > DOUBLE_TOLERANCE) {pVec[0]/=len;pVec[1]/=len;pVec[2]/=len;}	return len;	}
	bool      IsInfinite()	{   return ( pVec[0] == 1e300 && pVec[1] == 1e300 && pVec[2] == 1e300 );  }
	CVector3D XY()			{	return CVector3D( pVec[0], pVec[1], 0.0 );	}

	double & operator[]( const int i ) { return pVec[i]; }

	CVector3D& operator=(const CVector3D& v);
	CVector3D& operator+=(const CVector3D& v);
	CVector3D& operator-=(const CVector3D& v);
	CVector3D& operator*=(double u);
	CVector3D& operator/=(double u);
	CVector3D& operator^=(const CVector3D& v);

	friend CVector3D operator+(const CVector3D& lv, const CVector3D& rv);
	friend CVector3D operator-(const CVector3D& lv, const CVector3D& rv);
	friend CVector3D operator*(const double u, const CVector3D& rv);
	friend CVector3D operator*(const CVector3D& lv, const double u);
	friend CVector3D operator/(const CVector3D& lv, const double u);
	friend double   operator*(const CVector3D& lv, const CVector3D& rv);
	friend CVector3D operator^(const CVector3D& lv, const CVector3D& rv);

public:
	static const CVector3D INFINITE_VECTOR3D;
};
