#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

// Globals
struct s {
	float value;
	int pad[NUMPAD];
} Array[4];

// Function Prototypes
void fix1();
void fix2();

int main( int argc, char *argv[] )
{
	fix1();
	// fix2();
}

/**
 * Measures attempts to fix false sharing through padding
 */
void fix1() {
	omp_set_num_threads(NUMT);
	const int SomeBigNumber = 100000000;	// keep < 2B

	double startTime = omp_get_wtime();
	#pragma omp parallel for
	for (int i = 0; i < 4; i++) {
		unsigned int seed = 0;
		for (unsigned int j = 0; j < SomeBigNumber; j++) {
			Array[i].value = Array[i].value + (float)rand_r(&seed);
		}
	}
	double endTime = omp_get_wtime();
	printf("NUMT: %i | NUMPAD: %i\n", NUMT, NUMPAD);
	fprintf(stderr, "MegaAdds/sec = %10.2lf\n\n", (double)SomeBigNumber/(endTime-startTime)/1000000.);
}

/**
 * Measures attempts to fix false sharing through private variables
 */
void fix2() {
	omp_set_num_threads(NUMT);
	const int SomeBigNumber = 100000000;	// keep < 2B

	double startTime = omp_get_wtime();
	#pragma omp parallel for
	for (int i = 0; i < 4; i++) {
		unsigned int seed = 0;
		float temp = Array[i].value;
		for (unsigned int j = 0; j < SomeBigNumber; j++) {
			temp = temp + (float)rand_r(&seed);
		}
		Array[i].value = temp;
	}
	double endTime = omp_get_wtime();
	printf("NUMT: %i | NUMPAD: %i\n", NUMT, NUMPAD);
	fprintf(stderr, "MegaAdds/sec = %10.2lf\n\n", (double)SomeBigNumber/(endTime-startTime)/1000000.);
}
