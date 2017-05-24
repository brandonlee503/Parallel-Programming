#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include "simd.p5.h"

// Globals
float ARRAY_A[ARRAY_SIZE];
float ARRAY_B[ARRAY_SIZE];
float ARRAY_C[ARRAY_SIZE];

// Function Prototypes
float Ranf( float low, float high );
double SIMD_SSE_MUL();
double SIMD_SSE_SUM();
double MUL();
double SUM();

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
	// Initialize with random values
	for (int i = 0; i < ARRAY_SIZE; i++) {
		ARRAY_A[i] = Ranf(-1.f, 1.f);
		ARRAY_B[i] = Ranf(-1.f, 1.f);
	}

	// Write to file
	std::ofstream outputFile;
	outputFile.open("outputFile.csv", std::ios_base::app);

	// Perform multiplications
	double sseMul = SIMD_SSE_MUL();
	double sseSum = SIMD_SSE_SUM();
	double mul = MUL();
	double sum = SUM();
	printf("\n");
	outputFile << ARRAY_SIZE << "," << sseMul/mul << "," << sseSum/sum << std::endl;
}

/**
 * Utilizes SIMD SSE to multiply two arrays
 */
double SIMD_SSE_MUL() {
	double startTime = omp_get_wtime();
	SimdMul(ARRAY_A, ARRAY_B, ARRAY_C, ARRAY_SIZE);
	double endTime = omp_get_wtime();
	fprintf(stderr, "SIMD_SSE_MUL MegaMults/sec = %10.2lf\n", (double)ARRAY_SIZE/(endTime-startTime)/1000000.);
	return (double)ARRAY_SIZE/(endTime-startTime)/1000000.;
}

/**
 * Utilizes SIMD SSE to multiply and reduce two arrays
 */
double SIMD_SSE_SUM() {
	double startTime = omp_get_wtime();
	SimdMulSum(ARRAY_A, ARRAY_B, ARRAY_SIZE);
	double endTime = omp_get_wtime();
	fprintf(stderr, "SIMD_SSE_SUM MegaMults/sec = %10.2lf\n", (double)ARRAY_SIZE/(endTime-startTime)/1000000.);
	return (double)ARRAY_SIZE/(endTime-startTime)/1000000.;
}

/**
 * Utilizes OMP to multiply two arrays
 */
double MUL() {
	double startTime = omp_get_wtime();
	#pragma omp simd
	for (int i = 0; i < ARRAY_SIZE; i++) {
		ARRAY_C[i] = ARRAY_A[i] * ARRAY_B[i];
	}
	double endTime = omp_get_wtime();
	fprintf(stderr, "MUL MegaMults/sec = %10.2lf\n", (double)ARRAY_SIZE/(endTime-startTime)/1000000.);
	return (double)ARRAY_SIZE/(endTime-startTime)/1000000.;
}

/**
 * Multiply and reduce two arrays
 */
double SUM() {
	double startTime = omp_get_wtime();
	for (int i = 0; i < ARRAY_SIZE; i++) {
		ARRAY_C[i] = ARRAY_A[i] * ARRAY_B[i] + ARRAY_B[i];
	}
	double endTime = omp_get_wtime();
	fprintf(stderr, "SUM MegaMults/sec = %10.2lf\n", (double)ARRAY_SIZE/(endTime-startTime)/1000000.);
	return (double)ARRAY_SIZE/(endTime-startTime)/1000000.;
}
