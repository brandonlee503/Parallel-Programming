#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

// Define Globals
#define SIZE    32768
float Array[2*SIZE];
float Sums[1*SIZE];

// Function Prototypes
void mp_parallelism();

int main(int argc, char const *argv[]) {

    // Open signal file
    FILE *fp = fopen("signal.txt", "r");

    if (fp == NULL) {
        fprintf(stderr, "Cannot open file 'signal.txt'\n");
        exit(1);
    }

    int Size;
    fscanf(fp, "%d", &Size);
    Size = SIZE;
    for (int i = 0; i < Size; i++) {
        fscanf(fp, "%f", &Array[i]);
        Array[i+Size] = Array[i];   // duplicate the array
    }
    fclose(fp);

    mp_parallelism();

    return 0;
}

/**
 * Performs operations with openMP
 */
void mp_parallelism() {
    double startTime = omp_get_wtime();
    #pragma omp parallel for
    for (int shift = 0; shift < SIZE; shift++) {
        float sum = 0.;
        for (int i = 0; i < SIZE; i++) {
            sum += Array[i] * Array[i + shift];
        }
        Sums[shift] = sum; // note the "fix #2" from false sharing if you are using OpenMP
    }
    double endTime = omp_get_wtime();
    printf("NUMT: %i\n", NUMT);
    fprintf(stderr, "MegaAdds/sec = %10.2lf\n\n", (double)SIZE/(endTime-startTime)/1000000.);
}
