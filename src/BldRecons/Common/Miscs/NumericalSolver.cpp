#include "StdAfx.h"
#include "NumericalSolver.h"
#include <math.h>

CNumericalSolver::CNumericalSolver(void)
{
}

CNumericalSolver::~CNumericalSolver(void)
{
}

#define ROTATE(e,i,j,k,l) g=e[i][j];h=e[k][l];e[i][j]=g-s*(h+g*tau);e[k][l]=h+s*(g-h*tau);

void CNumericalSolver::SolveEigenVectors2( double e[][2], double v[][2], double d[] )
{
	double b[2];
	double z[2];
	double tresh;

	for ( int i = 0; i < 2; i++ )
		for ( int j = 0; j < 2; j++ )
			v[i][j] = 0.0;
	v[0][0] = v[1][1] = 1.0;
	for ( int i = 0; i < 2; i++ ) {
		b[i] = d[i] = e[i][i];
		z[i] = 0.0;
	}

	for ( int k = 0; k < 50; k++ ) {
		double sum = 0.0;
		for ( int i = 0; i < 2; i++ )
			for ( int j = i + 1; j < 2; j++ )
				sum += fabs( e[i][j] );
		if ( sum == 0.0 )
			break;

		if ( k < 3 )
			tresh = 0.2 * sum / 9.0;
		else 
			tresh = 0.0;

		for ( int i = 0; i < 2; i++ )
			for ( int j = i + 1; j < 2; j++ ) {
				double g = 100.0 * fabs( e[i][j] );
				if ( k > 3 && fabs( d[i] ) + g == fabs( d[i] ) && fabs( d[j] ) + g == fabs( d[j] ) )
					e[i][j] = 0.0;
				else if ( fabs( e[i][j] ) > tresh ) {
					double h = d[j] - d[i];
					double tau, t, c, s;
					if ( fabs( h ) + g == fabs( h ) )
						t = e[i][j] / h;
					else {
						double theta = 0.5 * h / e[i][j];
						t = 1.0 / ( fabs( theta ) + sqrt( 1.0 + theta * theta ) );
						if ( theta < 0.0 ) t = -t;
					}
					c = 1.0 / sqrt( 1 + t * t );
					s = t * c;
					tau = s / ( 1.0 + c );
					h = t * e[i][j];
					z[i] -= h;
					z[j] += h;
					d[i] -= h;
					d[j] += h;
					e[i][j] = 0.0;
					for ( int l = 0; l < i; l++ ) {
						ROTATE( e, l, i, l, j );
					}
					for ( int l = i + 1; l < j; l++ ) {
						ROTATE( e, i, l, l, j );
					}
					for ( int l = j + 1; l < 2; l++ ) {
						ROTATE( e, i, l, j, l );
					}
					for ( int l = 0; l < 2; l++ ) {
						ROTATE( v, l, i, l, j );
					}
				}
			}

			for ( int i = 0; i < 2; i++ ) {
				b[i] += z[i];
				d[i] = b[i];
				z[i] = 0.0;
			}
	}

	double p;
	int q;
	for ( int i = 0; i < 2; i++ ) {
		q = i;
		p = d[ i ];
		for ( int j = i + 1; j < 2; j++ ) {
			if ( d[ j ] > p ) {
				q = j;
				p = d[ j ];
			}
		}
		if ( q != i ) {
			d[ q ] = d[ i ];
			d[ i ] = p;
			for ( int j = 0; j < 2; j++ ) {
				p = v[ j ][ i ];
				v[ j ][ i ] = v[ j ][ q ];
				v[ j ][ q ] = p;
			}
		}
	}
}

void CNumericalSolver::SolveEigenVectors3( double e[][3], double v[][3], double d[] )
{
	double b[3];
	double z[3];
	double tresh;

	for ( int i = 0; i < 3; i++ )
		for ( int j = 0; j < 3; j++ )
			v[i][j] = 0.0;
	v[0][0] = v[1][1] = v[2][2] = 1.0;
	for ( int i = 0; i < 3; i++ ) {
		b[i] = d[i] = e[i][i];
		z[i] = 0.0;
	}

	for ( int k = 0; k < 50; k++ ) {
		double sum = 0.0;
		for ( int i = 0; i < 3; i++ )
			for ( int j = i + 1; j < 3; j++ )
				sum += fabs( e[i][j] );
		if ( sum == 0.0 )
			break;

		if ( k < 3 )
			tresh = 0.2 * sum / 9.0;
		else 
			tresh = 0.0;

		for ( int i = 0; i < 3; i++ )
			for ( int j = i + 1; j < 3; j++ ) {
				double g = 100.0 * fabs( e[i][j] );
				if ( k > 3 && fabs( d[i] ) + g == fabs( d[i] ) && fabs( d[j] ) + g == fabs( d[j] ) )
					e[i][j] = 0.0;
				else if ( fabs( e[i][j] ) > tresh ) {
					double h = d[j] - d[i];
					double tau, t, c, s;
					if ( fabs( h ) + g == fabs( h ) )
						t = e[i][j] / h;
					else {
						double theta = 0.5 * h / e[i][j];
						t = 1.0 / ( fabs( theta ) + sqrt( 1.0 + theta * theta ) );
						if ( theta < 0.0 ) t = -t;
					}
					c = 1.0 / sqrt( 1 + t * t );
					s = t * c;
					tau = s / ( 1.0 + c );
					h = t * e[i][j];
					z[i] -= h;
					z[j] += h;
					d[i] -= h;
					d[j] += h;
					e[i][j] = 0.0;
					for ( int l = 0; l < i; l++ ) {
						ROTATE( e, l, i, l, j );
					}
					for ( int l = i + 1; l < j; l++ ) {
						ROTATE( e, i, l, l, j );
					}
					for ( int l = j + 1; l < 3; l++ ) {
						ROTATE( e, i, l, j, l );
					}
					for ( int l = 0; l < 3; l++ ) {
						ROTATE( v, l, i, l, j );
					}
				}
			}

		for ( int i = 0; i < 3; i++ ) {
			b[i] += z[i];
			d[i] = b[i];
			z[i] = 0.0;
		}
	}

	double p;
	int q;
	for ( int i = 0; i < 3; i++ ) {
		q = i;
		p = d[ i ];
		for ( int j = i + 1; j < 3; j++ ) {
			if ( d[ j ] > p ) {
				q = j;
				p = d[ j ];
			}
		}
		if ( q != i ) {
			d[ q ] = d[ i ];
			d[ i ] = p;
			for ( int j = 0; j < 3; j++ ) {
				p = v[ j ][ i ];
				v[ j ][ i ] = v[ j ][ q ];
				v[ j ][ q ] = p;
			}
		}
	}
}
