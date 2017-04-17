#include <omp.h>
#include <stdio.h>
#include <math.h>

#define NUMT	        1
#define ARRAYSIZE       1024*2024
#define NUMTRIES        10

float A[ARRAYSIZE];
float B[ARRAYSIZE];
float C[ARRAYSIZE];

int main( )
{

//check if OPENMP exists
#ifndef _OPENMP
        fprintf( stderr, "OpenMP is not supported here -- sorry.\n" );
        return 1;
#endif

        omp_set_num_threads( NUMT );				//set number of threads
        fprintf( stderr, "Using %d threads\n", NUMT );

        double maxMegaMults = 0.;
        double sumMegaMults = 0.;

        for( int t = 0; t < NUMTRIES; t++ )			
        {
                double time0 = omp_get_wtime( );

 
		//split into NUMTRIES of threads and go back to one thread after for loop is done		 executing
               #pragma omp parallel for
                for( int i = 0; i < ARRAYSIZE; i++ )
                {
                        C[i] = A[i] * B[i];
                }

                double time1 = omp_get_wtime( );
                double megaMults = (double)ARRAYSIZE/(time1-time0)/1000000.;
                sumMegaMults += megaMults;
                if( megaMults > maxMegaMults )
                        maxMegaMults = megaMults;
        }

        double avgMegaMults = sumMegaMults/(double)NUMTRIES;
        printf( "   Peak Performance = %8.2lf MegaMults/Sec\n", maxMegaMults );
        printf( "Average Performance = %8.2lf MegaMults/Sec\n", avgMegaMults );

	return 0;
}
