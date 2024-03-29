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
   //printInfo(disk); // debugging purpose

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
   iNode_info *second_last_info = step_to_target(disk, fd, argv[2], 1);
   if (second_last_info == NULL) {
      fprintf(stderr, "! Invalid path !\n");
      return ENOENT;
   }
   struct ext2_inode *second_last_inode = second_last_info->iNode;
   int second_last_inode_number = second_last_info->iNode_number;

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
            if (match_name(argv[2], file_name, iLastDir, iLastChar)) {
               fprintf(stderr, "Directory \"%s\" trying to create already exists\n", file_name);
               return ENOENT;
            }
      }
      // Else update relavent index to keep checking the rest of the files
      size += entry->rec_len; // update size we have read so far
      entry = (void*) entry + entry->rec_len; // move to the next entry
   }
   // Function does not exit in the above loop --> path okay
   //printf("# Path check passed: input path okay #\n");


   /* The program has been tested till this point */


   /* --------- Create a directory with name being the last directory from argv[2] --------- */
   // Get the index of the first free inode for our new directory
   int iInode = find_free_inode(disk);
   if (iInode == -1) {
      fprintf(stderr, "Disk compact.");
      return ENOENT;
   }
   struct ext2_inode *newInode = get_inode(iInode + 1, disk);
   //printf("Inode free at index %d\n", iInode);
   
   // A new directory entry needs to be created under the second last parent directory
   // Need a free block inside the parent directory inode for this
   int new_name_len = iLastChar - iLastDir + 1;
   int newDirEtryNum = find_spot_for_inode_entry(second_last_inode_number, disk);
   struct ext2_dir_entry_2 *newDirEtry = (struct ext2_dir_entry_2 *)get_block(newDirEtryNum, disk); // TODO: implement in the helper function
   newDirEtry->file_type = EXT2_FT_DIR;
   newDirEtry->inode = iInode + 1;
   newDirEtry->rec_len = new_name_len + 8; // TODO: check if the rec_len here is fine.
   newDirEtry->name_len = new_name_len;
   memcpy(newDirEtry->name, argv[2]+iLastDir, new_name_len); // Set name of the entry
   set_block_bitmap(newDirEtryNum, disk);
   ///////
   //printf("Parent entry for inode in block number %d\n", newDirEtryNum);
   //printInfo(disk);
   // From the index we get the pointer to the inode & block
   //printf("newDirEtry index is: %d\n", newDirEtryNum - 1);
   //printf("%s\n", newDirEtry->name);
   int iBlock = find_free_block(disk);
   if (iBlock == -1) {
      fprintf(stderr, "Disk compact.");
      return ENOENT;
   }
   
   struct ext2_dir_entry_2 *newInodeBlk = (struct ext2_dir_entry_2 *)get_block(iBlock+1, disk);
   struct ext2_dir_entry_2 *newInodeBlk2 = newInodeBlk + 12; // Harded coded offset of 12
   /////////
   set_block_bitmap(iBlock + 1, disk);
   //printf("Inode's block free at index %d\n", iBlock);
   //printInfo(disk);
   // Set attributes in the inode and its directory entries
   (newInode->i_block)[0] = iBlock;
   newInode->i_mode = EXT2_S_IFREG;
   newInodeBlk->file_type = EXT2_FT_DIR;
   newInodeBlk->inode = iInode + 1;
   newInodeBlk->rec_len = 12; // Hard-coded length 12 for self directory entry "."
   newInodeBlk->name_len = 1;
   memset(newInodeBlk->name, '.', 1); // Set name of the entry

   newInodeBlk2->file_type = EXT2_FT_DIR;
   newInodeBlk2->inode = second_last_inode_number + 1;
   newInodeBlk2->rec_len = 12; // Hard-coded length 12 for parent directory entry ".."
   newInodeBlk2->name_len = 2; // The name ".." has length of 2
   memset(newInodeBlk2->name, '.', 2); // Set name of the entry

   bgd->bg_used_dirs_count ++;
   bgd->bg_free_inodes_count --;
   bgd->bg_free_blocks_count --;
   sb->s_free_inodes_count --;
   sb->s_free_blocks_count --;
   set_inode_bitmap(iInode + 1, disk);


   //printf("# Mkdir done #\n");

   //printInfo(disk);
   //printf("%s\n", newDirEtry->name);
   return 0;
}
