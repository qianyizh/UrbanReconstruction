// Vector2D.cpp: implementation of the Vector2D class.
//
//////////////////////////////////////////////////////////////////////

#include "Geometry.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Vector2D& Vector2D::operator=(const Vector2D& v)
{
	x = v.x;	y = v.y;
	return (*this);
}
Vector2D& Vector2D::operator+=(const Vector2D& v)
{
	x += v.x;	y += v.y;
	return (*this);
}
Vector2D& Vector2D::operator-=(const Vector2D& v)
{
	x -= v.x;	y -= v.y;
	return (*this);
}
Vector2D& Vector2D::operator*=(double u)
{	
	x *= u;		y *= u;
	return (*this);
}
Vector2D& Vector2D::operator/=(double u)
{
	if (!EQUALZERO(u))
	{
		x /= u;		y /= u;
	}
	return(*this);
}

Vector2D operator+(const Vector2D& lv, const Vector2D& rv)
{
	Vector2D rel = lv;
	rel += rv;
	return rel;
}


Vector2D operator-(const Vector2D& lv, const Vector2D& rv)
{
	Vector2D rel = lv;
	rel -= rv;
	return rel;
}

Vector2D operator*(const double u, const Vector2D& rv)
{
	Vector2D rel = rv;
	rel *= u;
	return rel;
}

Vector2D operator*(const Vector2D& lv, const double u)
{
	Vector2D rel = lv;
	rel *= u;
	return rel;
}

Vector2D operator/(const Vector2D& lv, const double u)
{
	Vector2D rel = lv;
	rel /= u;
	return rel;
}

double   operator*(const Vector2D& lv, const Vector2D& rv)
{
	return lv.x*rv.x + lv.y*rv.y;
}

short Vector2D::AtWhere(Vector2D v0,Vector2D v1)
{
	Vector2D vTemp1(v1.y-v0.y,v0.x-v1.x);
	Vector2D vTemp2(x-v0.x,y-v0.y);
	double d=vTemp1*vTemp2;
	if(EQUALZERO(d))
		return 0;
	if(d>0)//right
		return 1;
	return -1;
}

bool Vector2D::AtRight(Vector2D v0,Vector2D v1)
{
	if(AtWhere(v0,v1)==1)
		return true;
	return false;
}
bool Vector2D::AtLeft(Vector2D v0,Vector2D v1)
{
	if(AtWhere(v0,v1)==-1)
		return true;
	return false;
}
bool Vector2D::OnLine(Vector2D v0,Vector2D v1)
{
	if(AtWhere(v0,v1)==0)
		return true;
	return false;
}

bool Vector2D::Intersect(Vector2D v1,Vector2D v2)//intersect with uint circle
{
	Vector2D vOrigin;
	short s=vOrigin.AtWhere(v1,v2);

	Vector2D vTemp1=v2-v1;
	vTemp1.normalize();
	Vector2D vTemp2;
//	if(s==0)//pass point (0,0)
	if(s==1)//right
	{
		vTemp2.x=-vTemp1.y;
		vTemp2.y=vTemp1.x;
		double d=vTemp2*v1;
		vTemp2*=d;
		d=sqrt(1.0-d*d);
		vTemp1*=d;
	}
	else if(s==-1)//left
	{
		vTemp2.x=vTemp1.y;
		vTemp2.y=-vTemp1.x;
		double d=vTemp2*v1;
		vTemp2*=d;
		d=sqrt(1.0-d*d);
		vTemp1*=d;
	}
	x=vTemp1.x+vTemp2.x;
	y=vTemp1.y+vTemp2.y;
	return true;
}
bool Vector2D::Intersect(Vector2D v1,Vector2D v2,Vector2D v3,Vector2D v4)//tow line intersect
{
	double d=(v4.y-v3.y)*(v1.x-v2.x)-(v2.y-v1.y)*(v3.x-v4.x);
	if(EQUALZERO(d))
		return false;

	double d1=v1.x*v2.y-v2.x*v1.y;
	double d2=v3.x*v4.y-v4.x*v3.y;
	x=((v4.x-v3.x)*d1-(v2.x-v1.x)*d2)/d;
	y=((v4.y-v3.y)*d1-(v2.y-v1.y)*d2)/d;
	return true;
}
double Vector2D::GetArea(Vector2D v)
{
	return x*v.y-v.x*y;
}
/////////////////////////////////////////////////////////////
// Vector3D : 3D vector
/////////////////////////////////////////////////////////////
Vector3D& Vector3D::operator=(const Vector3D& v)
{	
	x = v.x;	y = v.y;	z = v.z;
	return (*this);
}
Vector3D& Vector3D::operator+=(const Vector3D& v)
{	
	x += v.x;	y += v.y;	z += v.z;	
	return (*this);	
}
Vector3D& Vector3D::operator-=(const Vector3D& v)
{	
	x -= v.x;	y -= v.y;	z -= v.z;	
	return (*this);	
}
Vector3D& Vector3D::operator*=(double u)
{	
	x *= u;		y *= u;		z *= u;		
	return (*this);	
}
Vector3D& Vector3D::operator/=(double u)
{	
	if (!EQUALZERO(u))
	{x /= u;		y /= u;		z /= u;}
	return(*this);
}
Vector3D& Vector3D::operator^=(const Vector3D& v)
{	
	double xx = y*v.z - z*v.y;	
	double yy = z*v.x - x*v.z;	
	double zz = x*v.y - y*v.x;	
	x = xx; y = yy; z = zz; 
	return (*this);	
}


Vector3D operator+(const Vector3D& lv, const Vector3D& rv)
{
	Vector3D rel = lv;
	rel += rv;
	return rel;
}


Vector3D operator-(const Vector3D& lv, const Vector3D& rv)
{
	Vector3D rel = lv;
	rel -= rv;
	return rel;
}

Vector3D operator*(const double u, const Vector3D& rv)
{
	Vector3D rel = rv;
	rel *= u;
	return rel;
}

Vector3D operator*(const Vector3D& lv, const double u)
{
	Vector3D rel = lv;
	rel *= u;
	return rel;
}

Vector3D operator/(const Vector3D& lv, const double u)
{
	Vector3D rel = lv;
	rel /= u;
	return rel;
}

double   operator*(const Vector3D& lv, const Vector3D& rv)
{
	return lv.x*rv.x + lv.y*rv.y + lv.z*rv.z;
}

Vector3D operator^(const Vector3D& lv, const Vector3D& rv)
{
	Vector3D rel = lv;
	rel ^= rv;
	return rel;
}