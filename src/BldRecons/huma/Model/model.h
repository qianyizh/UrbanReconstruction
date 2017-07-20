#ifndef _MODEL_H_
#define _MODEL_H_

#include "..\common\geometry.h"

class CModel
{
public:
	CModel() {}
public:
	Vector3D m_vTranslation;//偏移量
	Vector3D m_vRotation;//旋转
	double m_color[3];//物体的颜色

};

#endif