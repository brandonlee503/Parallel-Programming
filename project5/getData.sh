#!/bin/bash

g++ -c simd.p5.cpp -o simd.p5.o

for array_size in 1000 10000 100000 1000000
do
	g++ -DARRAY_SIZE=$array_size -o arraymult arraymult.cpp simd.p5.o -lm -fopenmp
	./arraymult
done

# Threads
# for thread in 1 2 4
# do
# 	# Performance (Scheduling Type, Chunk Size)
# 	for numpad in 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
# 	do
# 		g++ -DNUMT=$thread -DNUMPAD=$numpad main.cpp -o main -lm -fopenmp
# 		./main
# 	done
# done
