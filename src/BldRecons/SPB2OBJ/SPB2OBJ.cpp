// SPB2OBJ.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ParamManager.h"
#include "MeshGrid.h"

void main( int argc, char * argv[] )
{

	CParamManager::GetParamManager()->RegisterCommandLine( argc, argv );

	CMeshGrid grid;
	
	grid.Mesh();

}

