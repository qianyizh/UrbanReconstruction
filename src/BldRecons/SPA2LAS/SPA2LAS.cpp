// SPA2LAS.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ParamManager.h"
#include "Streaming\SPAReader.h"
#include "Grid\StreamingGrid.h"
#include "Lidar\LidarCommon.h"

#include "liblas\lasfile.hpp"
using namespace liblas;

void main( int argc, char * argv[] )
{
	CParamManager * manager = CParamManager::GetParamManager();
	manager->RegisterCommandLine( argc, argv );

	CStreamingGrid grid;

	CSPAReader reader;
	reader.OpenFile( manager->m_pInputFile );
	reader.RegisterGrid( &grid );
	reader.ReadHeader();

	unsigned int totalnum = ( unsigned int )( manager->m_dbSampleRate * grid.m_nPointNumber );

	LASHeader header;
	header.SetMin( grid.m_cBoundingBox.m_vMin[0], grid.m_cBoundingBox.m_vMin[1], grid.m_cBoundingBox.m_vMin[2] );
	header.SetMax( grid.m_cBoundingBox.m_vMax[0], grid.m_cBoundingBox.m_vMax[1], grid.m_cBoundingBox.m_vMax[2] );
	header.SetPointRecordsCount( totalnum );

	LASFile file( std::string( manager->m_pOutputFile ), header, LASFile::eWrite );
	LASWriter & writer = file.GetWriter();
	
	int mark = 0;
	int num = 0;
	int progress = 0;

	fprintf_s( stderr, "Converting progress ...   0%%" );

	bool bLoop = true;
	while ( bLoop ) {
		if ( reader.ReadNextElement() ) {		// read chunk information

			SPACell * cell = reader.GetCell();
			if ( cell->type == -1 ) {
				bLoop = false;
			}

		} else {

			SPAPoint * point = reader.GetPoint();
			num++;
			int temp = ( int )( num * manager->m_dbSampleRate );

			int new_progress = ( int )( num * 100.0 / grid.m_nPointNumber );
			if ( new_progress > progress ) {
				progress = new_progress;
				fprintf_s( stderr, "\b\b\b\b%3d%%", progress );
			}

			if ( temp > mark ) {
				mark = temp;

				if ( manager->m_bClip == false || 
					( ( point->pos[0] >= manager->m_dbClip[0][0] )
					&& ( point->pos[0] <= manager->m_dbClip[0][1] )
					&& ( point->pos[1] >= manager->m_dbClip[1][0] )
					&& ( point->pos[1] <= manager->m_dbClip[1][1] ) ) )
				{

					LASPoint pt;
					pt.SetX( point->pos[0] );
					pt.SetY( point->pos[1] );
					pt.SetZ( point->pos[2] );

					switch ( point->type ) {
					case PT_Noise:
						pt.SetClassification( 7 );
						break;
					case PT_Building:
						pt.SetClassification( 6 );
						break;
					case PT_Tree:
						pt.SetClassification( 4 );
						break;
					default:
						pt.SetClassification( 1 );
						break;
					}
					writer.WritePoint( pt );

				}

			}
		}
	}

	fprintf_s( stderr, " ... done.\n" );
}
