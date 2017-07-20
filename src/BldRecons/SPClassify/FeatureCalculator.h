#pragma once

#include "Lidar\LidarCommon.h"

class CFeatureCalculator
{
public:
	CFeatureCalculator(void);
	~CFeatureCalculator(void);

public:
	static void ComputeCoVariance( PointData & point, PointDataVector & data );
	static void ComputeNormalCoVariance( PointData & point, PointDataVector & data );
	static void RefineClassification( PointData & point, PointDataVector & data );
};
