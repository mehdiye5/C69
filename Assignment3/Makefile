sim : ext2_rm.o helper.o
	gcc -Wall -g -o sim $^

bin : ext2_rm.c helper.c
	gcc -Wall -g -c $^

clean:
	rm *.o sim