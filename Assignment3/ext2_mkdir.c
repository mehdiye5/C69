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


#define BASE_OFFSET 1024
#define BLOCK_OFFSET(block) (BASE_OFFSET + (block-1)*EXT2_BLOCK_SIZE)
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
   // int inodeCount = sb->s_inodes_count;
   // int blockCount = sb->s_blocks_count;
   // Pointer to the group descriptor
   struct ext2_group_desc *bgd = (struct ext2_group_desc *) (disk + 2*EXT2_BLOCK_SIZE);
   // int iBlockBitmap = bgd->bg_block_bitmap;
   // int iInodeBitmap = bgd->bg_inode_bitmap;
   // int iInodeTable = bgd->bg_inode_table;

   // Indices regards the input absolute path
   // 1. iLastChar is the index of the last character that is not '/'
   int iLastChar = strlen(argv[2]) - 1;
   if (argv[2][iLastChar] == '/') {
      iLastChar --;
   }
   // 2. iLastDir is the index of the first character of the last directory
   int iLastDir = iLastChar;
   while (argv[2][iLastDir] != '/') {
      iLastDir --;
   }
   iLastDir ++;
   // 3. iPathAnchor is the index of the current character of the input path we are looking at
   int iPathAnchor = 1; // skip the first slash '/'
   printf("iLastChar: %d, iLastDir: %d\n", iLastChar, iLastDir);
   // locate the root inode
   struct ext2_inode *currInode = get_inode(2, disk); // root inode by default is the second inode
   //struct ext2_inode *prevInode;


   /* -------- Step to the second last directory from the given directory in argv[2] -------- */
   while (1) {
      // The inode is a directory inode
      /* Check if the inode contains the desired name from input path */
      struct ext2_dir_entry_2 *entry;
      struct ext2_dir_entry_2 *found = NULL;
      unsigned int size;
      unsigned char block[EXT2_BLOCK_SIZE];
      // Read the first block of the current inode
      lseek(fd, BLOCK_OFFSET(currInode->i_block[0]), SEEK_SET);
      read(fd, block, EXT2_BLOCK_SIZE);
      size = 0; // Bytes read so far
      entry = (struct ext2_dir_entry_2 *) block; // entry is initialized to the first block of the current inode
      while(size < currInode->i_size) {
         // printf("in loop: %d\n", iPathAnchor);
         // printf("%s\n", entry->name);
         // Proceed name check only when entry is a directory
         if (entry->file_type == EXT2_FT_DIR) {
            // Get the file name of the ext2_dir_entry_2
            char file_name[EXT2_NAME_LEN+1];
            memcpy(file_name, entry->name, entry->name_len);
            file_name[entry->name_len] = 0; // null char at the end
            // Directory with the desired name matched
            if (match_name(argv[2], file_name, iPathAnchor, iLastChar-1)) {
               printf("# found file %s, inode %u #\n", file_name, entry->inode);
               printf("------------ next round ------------\n");
               found = entry;
               iPathAnchor = iPathAnchor + entry->name_len + 1; // Update anchor of the input path
               //printf("%10u %s\n", entry->inode, file_name);
               break;
            }
         }
         // Update relavent index
         entry = (void*) entry + entry->rec_len; // move to the next entry
         size += entry->rec_len; // update size we have read so far
         printf("size: %d, rec_len: %d\n", size, currInode->i_size);
      }
      // If we do not find any inode that: it is a directory inode and it match the name, then report
      if (found == NULL) {
         printf("Invalid path\n");
         return -1;
      }
      // If we reach the index of the last directory, exit the loop
      if (iPathAnchor == iLastDir) {
         printf("# Second last directory reached #\n");
         break;
      }
      // Update current inode to the new inode for the upcoming iteration
      currInode = get_inode(found->inode, disk);
      found = NULL; // Reset found
   }


   /* --------- Create a directory with name being the last directory from argv[2] --------- */
   
    

   // learn which properties need to be updated when a new directory is created

     return 0;
}
