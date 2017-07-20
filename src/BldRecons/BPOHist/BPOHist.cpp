// BPOHist.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "HistGrid.h"
#include "ParamManager.h"

void main( int argc, char * argv[] )
{

	CParamManager * manager = CParamManager::GetParamManager();
	manager->RegisterCommandLine( argc, argv );

	CHistGrid grid;

	grid.Init();

	grid.Count();

	grid.SaveToHist( manager->m_pOutputFile );

	// grid.Debug_SaveToTxt();

}
