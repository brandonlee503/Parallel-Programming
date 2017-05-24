#!/bin/bash

g++ -c simd.p5.cpp -o simd.p5.o

MAX_ARR_SIZE=1024*1024*32
for ((i = 1024; i <= MAX_ARR_SIZE; i *= 2))
do
	g++ -DARRAY_SIZE=$i -o arraymult arraymult.cpp simd.p5.o -lm -fopenmp
	./arraymult
done
