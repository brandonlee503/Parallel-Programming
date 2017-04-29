#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#define ARRAY_SIZE	32*1024

// Array Global
float ARRAY[ARRAY_SIZE];

// Function Prototypes
float Ranf( float low, float high );

float Ranf( float low, float high )
{
	float r = (float) rand();		// 0 - RAND_MAX
	return(   low  +  r * ( high - low ) / (float)RAND_MAX   );
}

int main( int argc, char *argv[] )
{
	omp_set_num_threads(NUMT);

    for (int i = 0; i < ARRAY_SIZE - 1; i++) {
        ARRAY[i] = Ranf(-1.f, 1.f);
    }

	float prod;
	double startTime = omp_get_wtime();
	#pragma omp parallel for schedule(SCHEDULING,CHUNK_SIZE),private(prod)
	for (int i = 0; i < ARRAY_SIZE-1; i++) {
		prod = 1;
		for (int j = 0; j < i; j++) {
			prod *= ARRAY[j];
		}
	}
	double endTime = omp_get_wtime();

 	// count of how many multiplications were done:
	long int numMuled = (long int)ARRAY_SIZE * (long int)(ARRAY_SIZE+1) / 2;
	fprintf(stderr, "MegaMults/sec = %10.2lf\n\n", (double)numMuled/(endTime-startTime)/1000000.);
}
