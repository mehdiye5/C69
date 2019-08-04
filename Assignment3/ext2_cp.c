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
This program takes three command line arguments. The first is the name of an ext2 formatted
virtual disk. The second is the path to a file on your native operating system, and the third is an absolute
path on your ext2 formatted disk. The program should work like cp, copying the file on your native file
system onto the specified location on the disk. If the specified file or target location does not exist, then
your program should return the appropriate error (ENOENT). Please read the specifications of ext2
carefully, some things you will not need to worry about (like permissions, gid, uid, etc.), while setting
other information in the inodes may be important (e.g., i_dtime).
 */

unsigned char *disk; // Pointer to the ext2 disk

int main ( int argc, char **argv ) {

   // Make sure the number of arguments is 2
   if (argc != 3) {
      fprintf(stderr, "Usage: mkdir <image file name> <absolute path on ext2 disk>\n");
      exit(1);
   }  
   
   // Open the image file
   int fd = open(argv[1], O_RDWR);

   // Load the desired disk
   disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
   if(disk == MAP_FAILED) {
	   perror("mmap");
	   exit(1);
   }
   printInfo(disk); // debugging purpose

   // Step the directory where the file will be copies
   iNode_info *last_info = step_to_target(disk, fd, argv[2], 0);
   if (last_info == NULL) {
        printf("! Invalid path !\n");
        exit(1);
   }
   struct ext2_inode *last_inode = last_info->iNode;
   int last_inode_number = last_info->iNode_number;

   

}
