/*
This is a helper module where the helper functions will be located, to be used by other programs.
 */

#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <string.h>
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
 * Function returns the block bitmap
 */
unsigned char *get_block_bitmap(unsigned char* disk) {
    return (unsigned char *) (disk + EXT2_BLOCK_SIZE*(get_blocks_group_descriptor(disk)->bg_block_bitmap));
}

/**
 * Function returns the inode bitmap
 */
unsigned char *get_inode_bitmap(unsigned char* disk) {
    return (unsigned char *) (disk + EXT2_BLOCK_SIZE*(get_blocks_group_descriptor(disk)->bg_inode_bitmap));
}

/**
function returns Inodes table block for the given image file
Note: inodes table is found in the blocks group descriptor bg_inode_table atribute
 */
 unsigned char  *get_inode_table(unsigned char* disk) {
     return (unsigned char *)(disk + EXT2_BLOCK_SIZE*(get_blocks_group_descriptor(disk)->bg_inode_table));
 }

 /**
 function return the inode for the given inode number
  */
  struct ext2_inode *get_inode(int inode_number, unsigned char* disk) {
      return (struct ext2_inode*) (get_inode_table(disk) + sizeof(struct ext2_inode)*(inode_number - 1));
  }

char* get_file_name(char* directory) {
    char* copy = strdup(directory);    
    return basename(copy);
}

/**
 * Function that parse the input path by '/', and return an array of strings,
 * each represents a directory name from the input path
 */
char (*parse_absolute_path(char* inpPath))[EXT2_NAME_LEN] {
    char *currChar = inpPath + 1; // Skip the first root directory slash '/'
    char currStr[EXT2_NAME_LEN];
    int iCurrStr = 0;
    char tempBox[10][EXT2_NAME_LEN];
    int iTempBox = 0;

    while (*currChar != '\0') {
        if (*currChar == '/') {
            currStr[iCurrStr] = '\0';
            strcpy(tempBox[iTempBox], currStr);
            iCurrStr = 0;
            iTempBox ++;
            currChar ++;
            continue;
        }
        currStr[iCurrStr] = *currChar;
        iCurrStr ++;
        currChar ++;
    }
    if (iCurrStr != 0) {
        currStr[iCurrStr] = '\0';
        strcpy(tempBox[iTempBox], currStr);
        iTempBox ++;
    }
    char (*resultBox)[EXT2_NAME_LEN] = malloc((iTempBox) * sizeof(char[EXT2_NAME_LEN]));
    for (int i = 0; i < iTempBox; i ++) {
        strcpy(resultBox[i], tempBox[i]);
    }
    strcpy(resultBox[iTempBox], "\\");
    
    return resultBox;
}

