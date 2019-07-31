/*
This is a helper module where the helper functions will be located, to be used by other programs.
 */

 #include <stdio.h>
 #include "ext2.h"
 

 /**
 function return root inode number
  */
 int get_root_inode_number() {
     return EXT2_ROOT_INO;
 }

/**
function returns blocks group descriptor for the given image file
 */
 struct ext2_group_desc *get_blocks_group_descriptor(unsigned char* disk) {
     return (struct ext2_group_desc *)( disk + EXT2_BLOCK_SIZE*2);
 }

/**
function returns Inodes table block for the given image file
Note: inodes table is found in the blocks group descriptor bg_inode_table atribute
 */
 unsigned char  *get_inode_table(unsigned char* disk){
     return (unsigned char *)(disk + EXT2_BLOCK_SIZE*(get_blocks_group_descriptor(disk)->bg_inode_table);
 }

 /**
 function return the inode for the given inode number
  */
  struct ext2_inode *get_inode(int inode_number, unsigned char* disk) {
      return get_inode_table(disk) + (inode_number - 1);
  }

