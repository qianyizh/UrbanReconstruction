// LASFinalize.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ParamManager.h"
#include "FinalizeGrid.h"

#include "Miscs\TimeMeter.h"

void main(int argc, char * argv[])
{
	CTimeMeter timer;
	timer.Start();

	CParamManager::GetParamManager()->RegisterCommandLine( argc, argv );

	CFinalizeGrid grid;

	grid.ComputeBoundingBox();
	grid.Stream_ComputeGridIndex();
	grid.Stream_WriteGrid();

	timer.End();
	fprintf_s( stderr, "Total time cost is " );
	timer.Print();
	fprintf_s( stderr, ".\n" );
}
