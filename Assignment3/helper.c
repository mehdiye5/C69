/*
This is a helper module where the helper functions will be located, to be used by other programs.
 */

#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include "ext2.h"
#include "helper.h"
 

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

int match_name(char *path, char* actual, int start, int last) {
    //printf("start: %d, last: %d \n", start, last);
    while (*actual != '\0') {
        // No more character in path to compare with actual
        if (start >= last) {
            return 0;
        }
        // Mismatach between two strings
        if (path[start] != *actual) {
            return 0;
        }
        start ++;
        actual ++;
    }
    return 1;
}


Three_indices generate_position(char *path) {
    // Indices regards the input absolute path
    // 1. iLastChar is the index of the last character that is not '/'
    int iLastChar = strlen(path) - 1;
    if (path[iLastChar] == '/') {
        iLastChar --;
    }
    // 2. iLastDir is the index of the first character of the last directory
    int iLastDir = iLastChar;
    while (path[iLastDir] != '/') {
        iLastDir --;
    }
    iLastDir ++;
    // 3. iPathAnchor is the index of the current character of the input path we are looking at
    int iPathAnchor = 1; // skip the first slash '/'
    printf("iLastChar: %d, iLastDir: %d\n", iLastChar, iLastDir);
    Three_indices result = {
        .anchor = iPathAnchor,
        .last_char = iLastChar,
        .last_dir = iLastDir};
    return result;
}


/**
 * Function that step to the second last directory from the given directory
 */
struct ext2_inode *step_to_second_last(unsigned char* disk, int fd, char *path) {
    Three_indices indices = generate_position(path);
    int iPathAnchor = indices.anchor;
    int iLastChar = indices.last_char;
    int iLastDir = indices.last_dir;
    
    // locate the root inode
    struct ext2_inode *currInode = get_inode(2, disk); // root inode by default is the second inode

    while (1) {
        // The inode is a directory inode
        // Check if the inode contains the desired name from input path //
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
                if (match_name(path, file_name, iPathAnchor, iLastChar-1)) {
                    printf("# found file %s, inode %u #\n", file_name, entry->inode);
                    printf("------------ next round ------------\n");
                    found = entry;
                    currInode = get_inode(found->inode, disk);
                    iPathAnchor = iPathAnchor + entry->name_len + 1; // Update anchor of the input path
                    //printf("%10u %s\n", entry->inode, file_name);
                    break;
                }
            }
            // Update relavent index
            size += entry->rec_len; // update size we have read so far
            entry = (void*) entry + entry->rec_len; // move to the next entry
            
            printf("size: %d, rec_len: %d\n", size, currInode->i_size);
        }
        // If we do not find any inode that: it is a directory inode and it match the name, then complain
        if (found == NULL) {
            printf("Invalid path\n");
            return NULL;
        }
        // If we reach the index of the last directory, exit the loop
        if (iPathAnchor == iLastDir) {
            printf("# Valid path: second last directory reached #\n");
            return currInode;
        }
        // Update current inode to the new inode for the upcoming iteration
        found = NULL; // Reset found
    }

}


