sim : ext2_rm.o helper.o
	gcc -Wall -g -o sim $^

bin : ext2_rm.c helper.c
	gcc -Wall -g -c $^

binMkdir : ext2_mkdir.c helper.c
	gcc -Wall -g -c $^

mkdir: ext2_mkdir.o helper.o
	gcc -Wall -g -o ext2_mkdir $^

binCp : ext2_cp.c helper.c
	gcc -Wall -g -c $^

cp: ext2_cp.o helper.o
	gcc -Wall -g -o ext2_cp $^

clean:
	rm *.o sim