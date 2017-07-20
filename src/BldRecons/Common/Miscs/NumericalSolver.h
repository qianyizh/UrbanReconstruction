#pragma once

class CNumericalSolver
{
public:
	CNumericalSolver(void);
	~CNumericalSolver(void);

public:
	static void SolveEigenVectors3( double e[][3], double v[][3], double d[] );
	static void SolveEigenVectors2( double e[][2], double v[][2], double d[] );
};
