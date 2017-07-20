// DualContouring.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ParamManager.h"
#include "DCGrid.h"
#include "DCContourer.h"
#include "HistGrid.h"

void main(int argc, char * argv[])
{

	CParamManager * manager = CParamManager::GetParamManager();
	manager->RegisterCommandLine( argc, argv );
	DualContouringParam & param = manager->m_cDCParam;

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

				CPointCloud pointcloud;
				pointcloud.LoadFromBP( filename );

				CDCGrid grid;
				grid.AssignPointCloud( & pointcloud, param.m_dbGridLength, pointcloud.m_dbGroundZ );
				grid.ComputeHermiteData( param.m_nAcceptNumber, param.m_dbRelativeDistance, param.m_dbRelativeZ );
				grid.DualContouringGeometry( param.m_dbWeight, param.m_dbErrorTolerance, param.m_dbSingularTolerance );

				char truename[ 1024 ];
				memset( truename, 0, 1024 );
				strncat_s( truename, 1024, filename, strlen( filename ) - 2 );
				//memset( filename, 0, 1024 );
				//sprintf_s( filename, "%sxyzn", truename );
				//pointcloud.SaveToXYZN( filename );
				//memset( filename, 0, 1024 );
				//sprintf_s( filename, "%shd", truename );
				//grid.HermiteData_SaveToTxt( filename );

				CDCContourer contourer;
				contourer.AssignDCGrid( & grid );
				contourer.Contouring( manager->m_bWallRectangle, manager->m_bAntiNonManifold );

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