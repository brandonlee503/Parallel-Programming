#include <iostream>
#include <stdlib.h>
#include <omp.h>
#include <fstream>
#include <stdio.h>
#include "SIMD/simd.p5.h"


// Define Globals
int SIZE = 32768;
float Array[2*32768];
float Sums[1*32768];


// Function Prototypes
void simd_parallelism(std::string file);

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

    simd_parallelism("simd_parallelism.csv");

    return 0;
}

/**
 * Performs autocorrelation with SIMD
 */
void simd_parallelism(std::string file) {
    FILE *fp = std::fopen(file.c_str(), "w");

    double peakTime = 1000;
    double avgTime = 0;
    int avgLoops = 10;

    for (int i = 0; i < avgLoops; i++) {
        double startTime = omp_get_wtime();
        for (int shift = 0; shift < SIZE; shift++) {
            Sums[shift] = SimdMulSum(Array, &Array[shift], SIZE);
        }
        double endTime = omp_get_wtime();
        avgTime += endTime - startTime;

        if (endTime - startTime < peakTime) {
            peakTime = endTime - startTime;
        }
    }

    avgTime /= avgLoops;

    // Print titles and sums
    std::fprintf(fp, "Threads,Peak Performance (Mega Adds Per Second), Average Performance (Mega Adds Per Second)\n");
    std::fprintf(fp, "%lf,%lf\n", (double)SIZE * SIZE / peakTime / 1000000, (double)SIZE * SIZE / avgTime / 1000000);

    std::fprintf(fp, "Index,Sum\n");
    for (int i = 0; i < SIZE; i++) {
        std::fprintf(fp, "%d,%f\n", i, Sums[i]);
    }

    std::fclose(fp);
}
