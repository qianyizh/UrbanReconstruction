// ManifoldContouring.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ParamManager.h"
#include "MCGrid.h"
#include "MCContourer.h"
#include "HistGrid.h"

void main(int argc, char * argv[])
{

	CParamManager * manager = CParamManager::GetParamManager();
	manager->RegisterCommandLine( argc, argv );
	ManifoldContouringParam & param = manager->m_cMCParam;

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
				char truename[ 1024 ];
				memset( truename, 0, 1024 );
				strncat_s( truename, 1024, filename, strlen( filename ) - 2 );

				CPointCloud pointcloud;
				pointcloud.LoadFromBP( filename );

				CMCGrid grid;
				grid.AssignPointCloud( & pointcloud, param.m_dbGridLength, pointcloud.m_dbGroundZ );
				grid.ComputeHermiteData( param.m_nAcceptNumber, param.m_dbRelativeDistance, param.m_dbRelativeZ );
				grid.DualContouringGeometry( param.m_dbWeight, param.m_dbErrorTolerance, param.m_dbSingularTolerance, 3 );

				//sprintf_s( filename, "%srgeo", truename );
				//grid.RenderFoldingPoints( filename, true, false );

				CMCContourer contourer;
				contourer.AssignMCGrid( & grid );
				contourer.Contouring( manager->m_bWallRectangle );

				if ( manager->m_bWallRectangle && manager->m_cSnappingParam.m_bEnableSnapping ) {
					CHistGrid histgrid;
					histgrid.Init( manager->m_cSnappingParam.m_pSPBFile, manager->m_cSnappingParam.m_pHistFile );
					histgrid.Simplify( contourer, manager->m_cSnappingParam.m_dbErrorTolerance, manager->m_cSnappingParam.m_dbMinimumLength );
				}

				sprintf_s( filename, "%sobj", truename );
				contourer.SaveToObj( filename );

				fprintf_s( stderr, "done.\n" );
			}

			not_finished = FindNextFile( handle, &finder );

		}

	}

	FindClose( handle );
}