#!/bin/bash

# progs in the traceprogs directory
for prog in simpleloop matmul blocked test
do
	# run progs on following memory sizes
	for memory_size in 50 100 150 200
	do
		# for each trace algorithms
		for trace in clock fifo lru opt rand
		do
			# run sim.c with following flags
			# check line 93 to 110 for flag references
			./sim -f traceprogs/$prog -s 3000 -a $trace -m $memory_size >> output.txt 
		done 
	done
done 

