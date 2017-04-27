#!/bin/bash

# Threads
for thread in 1 2 4 6 8 10 12 14 16
do
	# Performance (Scheduling Type, Chunk Size)
	for i in static,1 static,4096 dynamic,1 dynamic,4096
	do IFS=","
		set -- $i
		g++ -DNUMT=$thread -DSCHEDULING=$1 -DCHUNK_SIZE=$2 main.cpp -o main -lm -fopenmp
		./main
	done
done
