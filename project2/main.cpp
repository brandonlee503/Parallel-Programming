#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
// #include <math.h>

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

	float prod = 0;

	#pragma omp parallel for schedule(SCHEDULING,CHUNK_SIZE),private(prod)
	for (int i = 0; i < ARRAY_SIZE-1; i++) {
		for (int j = 0; j < i; j++) {
			prod *= ARRAY[j];
		}
	}

	// double startTime = omp_get_wtime();
	// // Height Evaluation
	// #pragma omp parallel for reduction(+:volume),private(iu,iv,height)
	// for( int i = 0; i < NUMNODES*NUMNODES; i++ )
	// {
	// 	iu = i % NUMNODES;
	// 	iv = i / NUMNODES;
	// 	height = Height(iu, iv);
	//
	// 	if (iu > 0 && iu < NUMNODES-1 && iv > 0 && iv < NUMNODES-1) {
	// 		// Full tile
	// 		volume += height * fullTileArea;
	// 	} else {
	// 		if ((iu == NUMNODES-1 || iu == 0) && (iv == NUMNODES-1 || iv == 0)) {
	// 			// Quarter
	// 			volume += height * quarterTileArea;
	// 		} else {
	// 			// Half
	// 			volume += height * halfTileArea;
	// 		}
	// 	}
	// }
	//
	// double endTime = omp_get_wtime();
	// double mflops = (double)NUMNODES*NUMNODES/(endTime-startTime)/1000000.;
	// printf("NUMNODES: %i\n", NUMNODES);
	// printf("NUMT: %i\n", NUMT);
	// printf("Total volume: %f\n", volume);
	// printf("Performance: %10.2lf MFLOPS\n", mflops);
	// printf("Elapsed time: %10.2lf microseconds\n\n", 1000000. * (endTime-startTime));
}
