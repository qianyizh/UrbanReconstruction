// SPClassify.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ParamManager.h"
#include "ClassifyGrid.h"

void main(int argc, char * argv[])
{
	CParamManager::GetParamManager()->RegisterCommandLine( argc, argv );

	CClassifyGrid grid;

	grid.Classify();
}
