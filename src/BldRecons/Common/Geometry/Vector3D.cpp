#include "StdAfx.h"
#include ".\Vector3D.h"

CVector3D& CVector3D::operator=(const CVector3D& v)
{	
	pVec[0] = v.pVec[0];	pVec[1] = v.pVec[1];	pVec[2] = v.pVec[2];
	return (*this);
}
CVector3D& CVector3D::operator+=(const CVector3D& v)
{	
	pVec[0] += v.pVec[0];	pVec[1] += v.pVec[1];	pVec[2] += v.pVec[2];	
	return (*this);	
}
CVector3D& CVector3D::operator-=(const CVector3D& v)
{	
	pVec[0] -= v.pVec[0];	pVec[1] -= v.pVec[1];	pVec[2] -= v.pVec[2];	
	return (*this);	
}
CVector3D& CVector3D::operator*=(double u)
{	
	pVec[0] *= u;		pVec[1] *= u;		pVec[2] *= u;		
	return (*this);	
}
CVector3D& CVector3D::operator/=(double u)
{	
	if (abs(u) > 1e-10)
	{pVec[0] /= u;		pVec[1] /= u;		pVec[2] /= u;}
	return(*this);
}
CVector3D& CVector3D::operator^=(const CVector3D& v)
{	
	double xx = pVec[1]*v.pVec[2] - pVec[2]*v.pVec[1];	
	double yy = pVec[2]*v.pVec[0] - pVec[0]*v.pVec[2];	
	double zz = pVec[0]*v.pVec[1] - pVec[1]*v.pVec[0];	
	pVec[0] = xx; pVec[1] = yy; pVec[2] = zz; 
	return (*this);	
}


CVector3D operator+(const CVector3D& lv, const CVector3D& rv)
{
	CVector3D rel = lv;
	rel += rv;
	return rel;
}


CVector3D operator-(const CVector3D& lv, const CVector3D& rv)
{
	CVector3D rel = lv;
	rel -= rv;
	return rel;
}

CVector3D operator*(const double u, const CVector3D& rv)
{
	CVector3D rel = rv;
	rel *= u;
	return rel;
}

CVector3D operator*(const CVector3D& lv, const double u)
{
	CVector3D rel = lv;
	rel *= u;
	return rel;
}

CVector3D operator/(const CVector3D& lv, const double u)
{
	CVector3D rel = lv;
	rel /= u;
	return rel;
}

double   operator*(const CVector3D& lv, const CVector3D& rv)
{
	return lv.pVec[0]*rv.pVec[0] + lv.pVec[1]*rv.pVec[1] + lv.pVec[2]*rv.pVec[2];
}

CVector3D operator^(const CVector3D& lv, const CVector3D& rv)
{
	CVector3D rel = lv;
	rel ^= rv;
	return rel;
}

const CVector3D CVector3D::INFINITE_VECTOR3D = CVector3D( 1e300, 1e300, 1e300 );