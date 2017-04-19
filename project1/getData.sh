#!/bin/bash

# Threads
for thread in 1 2 4 6
do
	# Subdivisions
	for node in 50 100 500 1000 5000 7500 10000 15000
	do
		g++ -DNUMT=$thread -DNUMNODES=$node main.cpp -o main -lm -fopenmp
		./main
	done
done
