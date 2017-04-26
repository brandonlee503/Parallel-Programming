all:
	#sh getData.sh
	g++ -DNUMT=4 -DSCHEDULING=static -DCHUNK_SIZE=1 main.cpp -o main -lm -fopenmp
	./main

clean:
	rm -r main
