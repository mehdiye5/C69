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
virtual disk. The second is an absolute path on your ext2 formatted disk. The program should work
like mkdir, creating the final directory on the specified path on the disk. If any component on the path to
the location where the final directory is to be created does not exist or if the specified directory already

exists, then your program should return the appropriate error (ENOENT or EEXIST). Again, please read
the specifications to make sure you're implementing everything correctly (e.g., directory entries should be
aligned to 4B, entry names are not null-terminated, etc.).
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

   /* --------------- Initialize relevant poitners and indices --------------- */
   // Pointer to the super block
   struct ext2_super_block *sb = (struct ext2_super_block *)(disk + EXT2_BLOCK_SIZE);
   struct ext2_group_desc *bgd = (struct ext2_group_desc *) (disk + 2*EXT2_BLOCK_SIZE);

   // Relevant indices
   Three_indices indices = generate_position(argv[2]);
   //int iPathAnchor = indices.anchor;
   int iLastChar = indices.last_char;
   int iLastDir = indices.last_dir;

   /* -------- Step to the second last directory from the given directory in argv[2] -------- */
   struct ext2_inode *second_last_inode = step_to_second_last(disk, fd, argv[2]);
   if (second_last_inode == NULL) {
      exit(1);
   }

   /* --------------------- Check if the directory creating exists --------------------------*/
   // Check one more round to see if the directory we are creating exists in the current directory inode
   struct ext2_dir_entry_2 *entry;
   unsigned int size;
   unsigned char block[EXT2_BLOCK_SIZE];
   // Read the first block of the current inode
   lseek(fd, BLOCK_OFFSET(second_last_inode->i_block[0]), SEEK_SET);
   read(fd, block, EXT2_BLOCK_SIZE);
   size = 0; // Bytes read so far
   entry = (struct ext2_dir_entry_2 *) block; // entry is initialized to the first block of the current inode
   while(size < second_last_inode->i_size) {
      // Proceed name check only when entry is a directory
      if (entry->file_type == EXT2_FT_DIR) {
            // Get the file name of the ext2_dir_entry_2
            char file_name[EXT2_NAME_LEN+1];
            memcpy(file_name, entry->name, entry->name_len);
            file_name[entry->name_len] = 0; // null char at the end
            // Directory with the desired name matched
            if (match_name(argv[2], file_name, iLastDir, iLastChar-1)) {
               printf("Directory \"%s\" trying to create already exists\n", file_name);
               exit(1);
            }
      }
      // Else update relavent index to keep checking the rest of the files
      size += entry->rec_len; // update size we have read so far
      entry = (void*) entry + entry->rec_len; // move to the next entry
      
      printf("size: %d, rec_len: %d\n", size, second_last_inode->i_size);
   }
   // Function does not exit in the above loop --> path okay
   printf("# Path check passed: input path okay #\n");
   
   /* --------- Create a directory with name being the last directory from argv[2] --------- */



   // learn which properties need to be updated when a new directory is created

   return 0;
}
