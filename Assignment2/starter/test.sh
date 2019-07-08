#!/bin/bash

for prog in simpleloop matmul blocked test
do
	for memory_size in 50 100 150 200
	do
		for trace in clock fifo lru opt rand
		do
			./sim -f traceprogs/$prog -s 3000 -a $trace -m $memory_size >> output.txt 
		done 
	done
done 

