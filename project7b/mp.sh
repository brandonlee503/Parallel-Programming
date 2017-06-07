#!/bin/bash

# Threads
for thread in 1 2 4 8 16 32 64 128 256 512 1024
do
	g++ -DNUMT=$thread mp.cpp -o mp -lm -fopenmp
	./mp
done
