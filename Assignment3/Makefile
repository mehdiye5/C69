sim : ext2_rm.o
	gcc -Wall -g -o sim $^

%.o : %.c helper.h ext2.h
	gcc -Wall -g -c $<

clean:
	rm -f *.o sim *~