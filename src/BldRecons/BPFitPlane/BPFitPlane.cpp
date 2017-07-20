// BPFitPlane.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ParamManager.h"
#include "BPCloud.h"
#include "BPGrid.h"

void main(int argc, char * argv[])
{

	CParamManager * manager = CParamManager::GetParamManager();
	manager->RegisterCommandLine( argc, argv );

	char pSearch[ 1024 ];
	sprintf_s( pSearch, 1024, "%s*.bp", manager->m_pWorkingDir );

	WIN32_FIND_DATA finder;
	HANDLE handle = FindFirstFile( pSearch, & finder );
	BOOL not_finished = ( handle != INVALID_HANDLE_VALUE );

	while ( not_finished ) {

		if ( strcmp( finder.cFileName, "." ) != 0 && strcmp( finder.cFileName, ".." ) != 0 ) {

			if ( ( finder.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) != FILE_ATTRIBUTE_DIRECTORY ) {

				fprintf_s( stderr, "Processing %s ... ", finder.cFileName );

				char filename[ 1024 ];
				memset( filename, 0, 1024 );
				sprintf_s( filename, 1024, "%s%s", manager->m_pWorkingDir, finder.cFileName );
				CBPCloud cloud;
				CBPGrid grid( & cloud );
				cloud.LoadFromBP( filename );

				grid.BuildGridIndex();
				grid.PlaneFitting_RegionGrow();
				grid.GenerateOutline();

				cloud.SaveToBP( filename );

				sprintf_s( filename, 1024, "%s%so", manager->m_pWorkingDir, finder.cFileName );
				grid.WriteOutline( filename );

				fprintf_s( stderr, "done.\n" );

			}

			not_finished = FindNextFile( handle, &finder );

		}

	}

	FindClose( handle );

}
