// SPSplit.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ParamManager.h"
#include "SplitGrid.h"
#include "RefineGrid.h"

void main(int argc, char * argv[])
{
	CParamManager::GetParamManager()->RegisterCommandLine( argc, argv );

	CSplitGrid grid;

	grid.Split();

	// CFixedPatchSet::GetFixedPatchSet()->Load( CParamManager::GetParamManager()->m_pTempSetFile );

	CRefineGrid rgrid;

	rgrid.Refine();
}
