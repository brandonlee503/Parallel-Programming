#!/bin/bash

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

g++ main.cpp -o main -lm -fopenmp
./main
