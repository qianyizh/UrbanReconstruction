// DualContouring.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "ParamManager.h"
#include "DCGrid.h"
#include "DCContourer.h"

void main( int argc, char * argv[])
{

	fprintf_s( stderr, "2.5D Dual Contouring Demo\n" );
	fprintf_s( stderr, "Author: Qian-Yi Zhou (qianyizh@usc.edu)\n");
	fprintf_s( stderr, "Version: 1.01, last updated on 10/5/2010.\n");
	fprintf_s( stderr, "Project page:\n  http://graphics.usc.edu/~qianyizh/projects/dualcontouring.html\n");
	fprintf_s( stderr, "\n" );
	fprintf_s( stderr, "========================= Processing ========================\n" );

	CParamManager * manager = CParamManager::GetParamManager();
	manager->RegisterCommandLine( argc, argv );
	DualContouringParam & param = manager->m_cDCParam;

	char pSearch[ 1024 ];
	sprintf_s( pSearch, 1024, "%s*.xyzn", manager->m_pWorkingDir );

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

				CPointCloud pointcloud;
				pointcloud.LoadFromXYZN( filename );

				CDCGrid grid;
				grid.AssignPointCloud( & pointcloud, param.m_dbGridLength, pointcloud.m_dbGroundZ );
				grid.ComputeHermiteData( param.m_nAcceptNumber, param.m_dbRelativeDistance, param.m_dbRelativeZ );
				grid.DualContouringGeometry( param.m_dbWeight, param.m_dbErrorTolerance, param.m_dbSingularTolerance );

				CDCContourer contourer;
				contourer.AssignDCGrid( & grid );
				contourer.Contouring();

				char truename[ 1024 ];
				memset( truename, 0, 1024 );
				strncat_s( truename, 1024, filename, strlen( filename ) - 4 );
				sprintf_s( filename, "%sobj", truename );
				contourer.SaveToObj( filename );

				fprintf_s( stderr, "done.\n" );

			}

			not_finished = FindNextFile( handle, &finder );

		}

	}

	FindClose( handle );

}
