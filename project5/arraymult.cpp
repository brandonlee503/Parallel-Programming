#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include "simd.p5.h"

// Globals
float ARRAY_A[ARRAY_SIZE];
float ARRAY_B[ARRAY_SIZE];
float ARRAY_C[ARRAY_SIZE];

// Function Prototypes
float Ranf( float low, float high );
void SIMD_SSE_MUL();
void SIMD_SSE_SUM();

/**
 * Get a random float between the bounds
 * @param  low  Lower bound
 * @param  high Upper bound
 * @return      Random float
 */
float Ranf( float low, float high )
{
	float r = (float) rand();		// 0 - RAND_MAX
	return( low + r * ( high - low ) / (float)RAND_MAX );
}

int main( int argc, char *argv[] )
{
	for (int i = 0; i < ARRAY_SIZE; i++) {
		ARRAY_A[i] = Ranf(-1.f, 1.f);
		ARRAY_B[i] = Ranf(-1.f, 1.f);
	}

	SIMD_SSE_MUL();
	// printf("\n");
	SIMD_SSE_SUM();
}

/**
 * Utilizes SIMD SSE to multiply two arrays
 */
void SIMD_SSE_MUL() {
	double startTime = omp_get_wtime();
	SimdMul(ARRAY_A, ARRAY_B, ARRAY_C, ARRAY_SIZE);
	double endTime = omp_get_wtime();
	fprintf(stderr, "SIMD_SSE_MUL MegaMults/sec = %10.2lf\n", (double)ARRAY_SIZE/(endTime-startTime)/1000000.);
	// printf("\n");
}

/**
 * Utilizes SIMD SSE to multiply and reduce two arrays
 */
void SIMD_SSE_SUM() {
	double startTime = omp_get_wtime();
	SimdMulSum(ARRAY_A, ARRAY_B, ARRAY_SIZE);
	double endTime = omp_get_wtime();
	fprintf(stderr, "SIMD_SSE_SUM MegaMults/sec = %10.2lf\n", (double)ARRAY_SIZE/(endTime-startTime)/1000000.);
	// printf("\n");
}
