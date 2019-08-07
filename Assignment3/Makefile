all: ext2_rm ext2_mkdir ext2_cp ext2_ls

ext2_rm : ext2_rm.o helper.o
	gcc -Wall -g -o ext2_rm $^

bin : ext2_rm.c helper.c
	gcc -Wall -g -c $^

ext2_mkdir : ext2_mkdir.o helper.o
	gcc -Wall -g -o ext2_mkdir $^

ban: ext2_mkdir.c helper.c
	gcc -Wall -g -c $^

ext2_cp : ext2_cp.o helper.o
	gcc -Wall -g -o ext2_cp $^

bon: ext2_cp.c helper.c
	gcc -Wall -g -c $^

ext2_ls : ext2_ls.o helper.o
	gcc -Wall -g -o ext2_ls $^

bun: ext2_ls.c helper.c
	gcc -Wall -g -c $^

clean:
	-rm *.o $(objects) ext2_rm ext2_mkdir ext2_cp ext2_ls