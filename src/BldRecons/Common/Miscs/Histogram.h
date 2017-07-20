#pragma once

#include <vector>

class CHistogram
{
public:
	CHistogram(void);
	CHistogram( int precision, double s_min, double s_max );
	~CHistogram(void);

public:
	int m_nPrecision;
	int m_nNumber;
	double m_dbSection[ 2 ];
	std::vector< int > m_vecHist;
	std::vector< double > m_vecSmoothHist;
	std::vector< double > m_vecPeak;

public:
	void Init( int precision, double s_min, double s_max );
	void Add( double value, bool rotate = true );
	void Add( int section );

	void Save( FILE * pFile );
	void Load( FILE * pFile );

public:
	void GetPeak( double sigma, double tolerance );

private:
	void Smooth( double sigma );
	double MinSmoothHist();
	double HistBarValue( int i );
};
