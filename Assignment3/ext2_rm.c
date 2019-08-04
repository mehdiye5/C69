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

/*
This program takes two command line arguments. The first is the name of an ext2 formatted
virtual disk, and the second is an absolute path to a file or link (not a directory) on that disk. The program
should work like rm, removing the specified file from the disk. If the file does not exist or if it is a directory,
then your program should return the appropriate error. Once again, please read the specifications of ext2
carefully, to figure out what needs to actually happen when a file or link is removed (e.g., no need to zero
out data blocks, must set i_dtime in the inode, removing a directory entry need not shift the directory
entries after the one being deleted, etc.)
__       __
  \(*_*)/
        
Bonus(5% extra): Implement an additional "-r" flag (after the disk image argument), which allows
removing directories as well. In this case, you will have to recursively remove all the contents of the
directory specified in the last argument. If "-r" is used with a regular file or link, then it should be ignored
(the ext2_rm operation should be carried out as if the flag had not been entered). If you decide to do the
bonus, make sure first that your ext2_rm works, then create a new copy of it and rename it to
ext2_rm_bonus.c, and implement the additional functionality in this separate source file.
 */

 /**
 Note: most reference to understand ext2.h was done through following url http://cs.smith.edu/~nhowe/262/oldlabs/ext2.html#locate_file
       
  */



// note: copied for readimage.c from provided files  from week8&9
//pointer to the nmap from the image from the argument 1
unsigned char *disk;

 int main ( int argc, char **argv ) {

     // changed something
     // changed seming again

     printf("%d \n", argc);

     // case if thee is incorrect number of arguments
     // note: copied for readimage.c from provided files  from week8&9
     if (argc != 3) {
         fprintf(stderr, "Usage: readimg <image file name>\n");
        exit(1);
     }  

    //opening an image file from the 1st argument
    // note: copied for readimage.c from provided files  from week8&9
    int fd = open(argv[1], O_RDWR);

    // note: copied for readimage.c from provided files  from week8&9
    // assign pointer to the nmap for the the given image
    disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(disk == MAP_FAILED) {
	perror("mmap");
	exit(1);
    }


    // note: copied for readimage.c from provided files  from week8&9
    struct ext2_super_block *sb = (struct ext2_super_block *)(disk + 1024);
    printf("Inodes: %d\n", sb->s_inodes_count);
    printf("Blocks: %d\n", sb->s_blocks_count); 


    //=============testing part=============
    struct ext2_inode *inode = get_inode(get_root_inode_number(), disk);
    

    

    struct ext2_dir_entry* entry = entry = (struct ext2_dir_entry *)(disk + EXT2_BLOCK_SIZE * inode->i_block[0]);
    printf("The directory name is: %s \n", entry->name);
    printf("Number of Blocks is: %d \n", inode->i_blocks);

    struct ext2_inode *inode2 = get_inode(entry->inode, disk);

    
   

    // unsigned char *end = (orig + EXT2_BLOCK_SIZE);

    // printf("The fist pos is: %s \n", pos);
    // printf("The fist end is: %s \n", end);
    
    // struct ext2_dir_entry* second = (struct ext2_dir_entry *) pos;

    // pos += second->rec_len;

    // printf("The second pos is: %s \n", pos);

    for (int i = 0; i < 12; i ++){

         int block_num = (inode->i_block)[i];

        unsigned char *orig = disk + (block_num * EXT2_BLOCK_SIZE);

        unsigned char *pos = orig;

        struct ext2_dir_entry_2* o = (struct ext2_dir_entry_2 *) pos;

        if (o->inode != 0) {
            int s = 0;
            while (pos != orig + EXT2_BLOCK_SIZE) {
                //(disk + EXT2_BLOCK_SIZE * inode2->i_block[i])
                struct ext2_dir_entry* second = (struct ext2_dir_entry *) pos;
                printf("The directory name is: %s \n", second->name);
                

                printf("file type is: %d \n", o->file_type);
                
                if ((o->file_type == EXT2_FT_DIR) && strcmp(second->name, ".") != 0 && strcmp(second->name, "..") != 0) {
                    printf("found \n");
                    inode = get_inode(o->inode, disk);
                    s++;
                }
            
                pos += second->rec_len;
            }

        }
        
         
        
    }
    
    printf("Number of Blocks is: %d \n", inode2->i_blocks);
    

    //char* local_file = "/foo/bar/baz.txt";

    // char* ts1 = strdup(local_file);
    // char* ts2 = strdup(local_file);

    // char* dir = dirname(ts1);
    // char* filename = basename(ts2);

    // printf("file directory is: %s \n", dir);
    // printf("file name is: %s \n", filename);


    //======================================



    //locate the root inode

    // check if the given directory in argv[2] is a directory or a file    

    // check if a file exists

    // remove the file

    // learn how to free the inode

    // learn which properties need to be updated when file is removed

     return 0;
 }
