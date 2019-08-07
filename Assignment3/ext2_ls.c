#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "ext2.h"
#include "helper.h"
#include <libgen.h>
#include <string.h>
#include <errno.h>

/*
This program takes two command line arguments. The first is the name of an ext2 formatted
virtual disk. The second is an absolute path on the ext2 formatted disk. The program should work like ls
-1 (that's number one "1", not lowercase letter "L"), printing each directory entry on a separate line. If the
flag "-a" is specified (after the disk image argument), your program should also print the . and .. entries.
In other words, it will print one line for every directory entry in the directory specified by the absolute
path. If the path does not exist, print "No such file or directory", and return an ENOENT. Directories
passed as the second argument may end in a "/" - in such cases the contents of the last directory in the
path (before the "/") should be printed (as ls would do). Additionally, the path (the last argument) may be
a file or link. In this case, your program should simply print the file/link name (if it exists) on a single line,
and refrain from printing the . and ...

./ext2_ls disk.img /path/to/dir
./ext2_ls disk.img  -a /path/to/dir
 */

//nmap pointer
unsigned char *disk;
//from readimage.c
extern void print_dir_contents(struct ext2_inode *inode, int i);

int main(int argc, char **argv) {

    //incorrect number of args
    // args should be 3 or 4 (includes flag -a)
    if (!(argc == 3 || argc == 4)){
        fprintf(stderr, "Usage: ext2_ls <image file name>\n");
        exit(1);
    }
    //open image file
    int fd = open(argv[1], O_RDWR);
    //
    disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(disk == MAP_FAILED) {
	    perror("mmap");
	    exit(1);
    }

    struct ext2_group_desc *bgd = (struct ext2_group_desc *)(disk + (EXT2_BLOCK_SIZE *  2));
    struct ext2_super_block *sb = (struct ext2_super_block *)(disk + EXT2_BLOCK_SIZE);
    struct ext2_inode *inode_table = (struct ext2_inode *)(disk + (EXT2_BLOCK_SIZE * bgd->bg_inode_table));

    // must use ext2_dir_entry_2 for assignment
    struct ext2_dir_entry_2 *d;

    //get path and check if it exists
    char *path = argv[2];

    char *inode = find_dir_inode(path, disk);
    int inode_num = ((struct ext2_dir_entry_2 *) inode) -> inode;


    struct ext2_inode *c = inode_table+(inode_num-1);

    // if the inode is a directory, print the directory contents
    // we are printing from the last directory
    if (c->i_mode & EXT2_S_IFDIR) {
        print_dir_contents(inode_table, inode_num-1);
    } else if (c->i_mode & EXT2_S_IFREG) {
        //it's a regular file
        printf("%s\n", path);
    } else {
        fprintf(stderr, "‘%s’: No such file or directory\n", path);	
	    exit(ENOENT);
    }

    return 0;
}


void print_dir_contents(struct ext2_inode *inode, int i){
	int c;
	struct ext2_inode *in = inode+i;
	
    //from readimage.c
	if(in->i_mode & EXT2_S_IFDIR){
		for (c = 0; c < 12 && in->i_block[c]; c++){ 
			int size = EXT2_BLOCK_SIZE;
            char type;
			struct ext2_dir_entry_2 *d = (struct ext2_dir_entry_2 *)(disk + (in->i_block[c] * EXT2_BLOCK_SIZE));

			while(size > 0 ){  
                //switch (d->file_type) {
        		//case EXT2_FT_REG_FILE: type = 'f'; break;
        		//case EXT2_FT_DIR:      type = 'd'; break;
        		//default:               type = '?'; break;
        		//}
				char name[d->name_len+1];
		 		strncpy(name, d->name, d->name_len);
		 		name[d->name_len] = '\0';
				printf("%s\n", name);
				size -= d->rec_len;
				d = (struct ext2_dir_entry_2 *)((char *) d+d->rec_len);
			}			
		}
	}
}
