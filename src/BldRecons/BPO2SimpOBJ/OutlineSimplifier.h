#pragma once

#include "Geometry\Outline.h"
#include "AuxOutline.h"
#include "ParamManager.h"

struct LineSearchStructure {
	CLine line;
	int s, e;
	bool valid;
};

class COutlineSimplifier
{
public:
	COutlineSimplifier(void);
	~COutlineSimplifier(void);

public:
	static int Simplify( COutline & outline, CAuxOutline & aux, std::vector< int > & vlist, double tolerance, CParamManager::LineFittingMode fitting_mode = CParamManager::LFM_DirectConnect );

private:
	static CLine LineFitting_DirectConnect( COutline & outline, std::vector< int > & vlist, int s, int e );
	static CLine LineFitting_LeastSquare( COutline & outline, std::vector< int > & vlist, int s, int e );
	static bool CheckTolerance( COutline & outline, std::vector< int > & vlist, int s, int e, CLine & line, double tolerance );
};
