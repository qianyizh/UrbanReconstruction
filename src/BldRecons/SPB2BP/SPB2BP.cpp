// SPB2BP.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ParamManager.h"
#include "ConvertGrid.h"

void main(int argc, char * argv[])
{
	CParamManager * manager = CParamManager::GetParamManager();
	manager->RegisterCommandLine( argc, argv );

	CConvertGrid grid;

	grid.Convert();
}
