// BPO2SimpOBJ.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ParamManager.h"
#include "HistGrid.h"

void main( int argc, char * argv[] )
{

	CParamManager * manager = CParamManager::GetParamManager();
	manager->RegisterCommandLine( argc, argv );

	CHistGrid grid;

	grid.Init();

	grid.Simplify();

}
