#include <iostream>
#include <stdlib.h>
#include <omp.h>
#include <fstream>
#include <stdio.h>


// Define Globals
int SIZE = 32768;
float Array[2*32768];
float Sums[1*32768];


// Function Prototypes
void mp_parallelism(int threads, std::string file);

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

    mp_parallelism(1, "1_mp_thread.csv");
    mp_parallelism(8, "8_mp_thread.csv");

    return 0;
}

/**
 * Performs autocorrelation with OpenMP
 */
void mp_parallelism(int threads, std::string file) {

    FILE *fp = std::fopen(file.c_str(), "w");

    omp_set_num_threads(threads);

    int avgLoops = 10;
    double avgTime = 0;
    double peakTime = 1000;

    for (int loops = 0; loops < avgLoops; loops++) {

        double startTime = omp_get_wtime();

        #pragma omp parallel for default(none) shared(SIZE, Array, Sums)
        for (int shift = 0; shift < SIZE; shift++) {
	        float sum = 0.;
	        for (int i = 0; i < SIZE; i++) {
		        sum += Array[i] * Array[i + shift];
	        }
	        Sums[shift] = sum;
        }

        double endTime = omp_get_wtime();
        if (endTime - startTime < peakTime) {
            peakTime = endTime - startTime;
        }

        avgTime += endTime - startTime;
    }

    avgTime /= avgLoops;

    // Print titles and sums
    std::fprintf(fp, "Threads,Peak Performance (Mega Adds Per Second), Average Performance (Mega Adds Per Second)\n");
    std::fprintf(fp, "%d,%lf,%lf\n", threads, (double)SIZE * SIZE / peakTime / 1000000, (double)SIZE * SIZE / avgTime / 1000000);

    std::fprintf(fp, "Index,Sum\n");
    for (int i = 0; i < SIZE; i++) {
        std::fprintf(fp, "%d,%f\n", i, Sums[i]);
    }

    std::fclose(fp);
}
