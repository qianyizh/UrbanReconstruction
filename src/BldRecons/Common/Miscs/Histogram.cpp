#include "StdAfx.h"
#include ".\histogram.h"
#include <math.h>

CHistogram::CHistogram(void)
{
	m_nPrecision = 0;
}

CHistogram::CHistogram( int precision, double s_min, double s_max )
{
	Init( precision, s_min, s_max );
}

CHistogram::~CHistogram(void)
{
}

void CHistogram::Init( int precision, double s_min, double s_max )
{
	m_dbSection[ 0 ] = s_min;
	m_dbSection[ 1 ] = s_max;

	m_nPrecision = precision;
	m_vecHist.resize( precision, 0 );
	m_nNumber = 0;
}

void CHistogram::Add( double value, bool rotate /* = true */ )
{
	if ( value < m_dbSection[ 0 ] || value >= m_dbSection[ 1 ] ) {
		if ( rotate ) {
			double length = m_dbSection[ 1 ] - m_dbSection[ 0 ];
			while ( value >= m_dbSection[ 1 ] )
				value -= length;
			while ( value < m_dbSection[ 0 ] )
				value += length;
		} else {
			return;
		}
	}

	double sub_length = ( m_dbSection[ 1 ] - m_dbSection[ 0 ] ) / m_nPrecision;
	int section = ( int )( ( value - m_dbSection[ 0 ] ) / sub_length );
	if ( section < 0 )
		section = 0;
	if ( section >= m_nPrecision )
		section = m_nPrecision - 1;
	Add( section );
}

void CHistogram::Add( int section )
{
	m_nNumber++;
	m_vecHist[ section ] ++;
}

void CHistogram::Save( FILE * pFile )
{
	fwrite( m_dbSection, sizeof( double ), 2, pFile );
	fwrite( & m_nNumber, sizeof( int ), 1, pFile );

	int peak_num = ( int )m_vecPeak.size();
	fwrite( & peak_num, sizeof( int ), 1, pFile );
	for ( int i = 0; i < peak_num; i++ ) {
		fwrite( & m_vecPeak[ i ], sizeof( double ), 1, pFile );
	}

	fwrite( & m_nPrecision, sizeof( int ), 1, pFile );
	for ( int i = 0; i < m_nPrecision; i++ ) {
		fwrite( & m_vecHist[ i ], sizeof( int ), 1, pFile );
	}
}

void CHistogram::Load( FILE * pFile )
{
	fread( m_dbSection, sizeof( double ), 2, pFile );
	fread( & m_nNumber, sizeof( int ), 1, pFile );

	int peak_num;
	fread( & peak_num, sizeof( int ), 1, pFile );
	m_vecPeak.clear();
	m_vecPeak.resize( peak_num, 0.0 );
	for ( int i = 0; i < peak_num; i++ ) {
		fread( & m_vecPeak[ i ], sizeof( double ), 1, pFile );
	}

	fread( & m_nPrecision, sizeof( int ), 1, pFile );
	m_vecHist.clear();
	m_vecHist.resize( m_nPrecision, 0 );
	for ( int i = 0; i < m_nPrecision; i++ ) {
		fread( & m_vecHist[ i ], sizeof( int ), 1, pFile );
	}
}

//////////////////////////////////////////////////////////////////////////
// functions for get peaks
//////////////////////////////////////////////////////////////////////////

void CHistogram::GetPeak( double sigma, double tolerance )
{
	Smooth( sigma );
	double min_smooth_hist = MinSmoothHist();

	m_vecPeak.clear();
	for ( int i = 0; i < m_nPrecision; i++ ) {
		if ( m_vecSmoothHist[ i ] >= m_vecSmoothHist[ ( i - 1 + m_nPrecision ) % m_nPrecision ] &&
			m_vecSmoothHist[ i ] > m_vecSmoothHist[ ( i + 1 ) % m_nPrecision ] &&
			m_vecSmoothHist[ i ] >= min_smooth_hist * tolerance )
		{
			m_vecPeak.push_back( HistBarValue( i ) );
		}
	}
}

void CHistogram::Smooth( double sigma )
{
	m_vecSmoothHist.clear();
	m_vecSmoothHist.resize( m_nPrecision, 0.0 );

	int normal_pdf_half_length = __max( 5, ( ( int )( sigma * 3.0 ) + 1 ) );
	std::vector< double > normal_pdf( normal_pdf_half_length * 2 + 1 );

	for ( int i = 0; i <= normal_pdf_half_length; i++ ) {
		normal_pdf[ normal_pdf_half_length - i ] = normal_pdf[ normal_pdf_half_length + i ] =
			1.0 / sigma / sqrt( 2.0 * 3.1415926535897932384626 ) * exp( -0.5 * i * i / sigma / sigma );
	}

	// convolution
	for ( int i = 0; i < m_nPrecision; i++ ) {
		for ( int j = 0; j < normal_pdf_half_length * 2 + 1; j++ ) {
			m_vecSmoothHist[ i ] += normal_pdf[ j ] * m_vecHist[ ( i + j - normal_pdf_half_length + m_nPrecision ) % m_nPrecision ];
		}
	}
}

double CHistogram::MinSmoothHist()
{
	double min_hist = 1e300;
	for ( int i = 0; i < m_nPrecision; i++ ) {
		if ( min_hist > m_vecSmoothHist[ i ] ) {
			min_hist = m_vecSmoothHist[ i ];
		}
	}
	return min_hist;
}

double CHistogram::HistBarValue( int i )
{
	return ( m_dbSection[ 0 ] + ( m_dbSection[ 1 ] - m_dbSection[ 0 ] ) / ( double )m_nPrecision * ( i + 0.5 ) );
}