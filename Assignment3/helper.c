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

/**
 function return the block for the given block index in the block bitmap
*/
struct ext2_dir_entry_2 *get_block(int block_number, unsigned char* disk) {
    struct ext2_super_block *sb = (struct ext2_super_block *)(disk + EXT2_BLOCK_SIZE);
    // number of inodes per block
    unsigned int inodes_per_block = EXT2_BLOCK_SIZE / sizeof(struct ext2_inode);
    // size in blocks of the inode table
    unsigned int itable_blocks = sb->s_inodes_per_group / inodes_per_block;
    return (struct ext2_dir_entry_2*) (get_inode_table(disk) + itable_blocks + EXT2_BLOCK_SIZE*(block_number - 1));
}


char* get_file_name(char* directory) {
    char* copy = strdup(directory);    
    return basename(copy);
}

int match_name(char *path, char* actual, int start, int last) {
    // printf("path: %s, actual: %s\n", path, actual);
    // printf("start: %d, last: %d\n", start, last);
    //printf("start: %d, last: %d \n", start, last);
    while (*actual != '\0') {
        // No more character in path to compare with actual
        if (start > last) {
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
    Three_indices result = {
        .anchor = iPathAnchor,
        .last_char = iLastChar,
        .last_dir = iLastDir};
    return result;
}


/**
 * Function that step to the second last directory from the given directory
 * - When flag is 0, it will step to the last directory
 * - When flag is 1, it will step to the second last directory
 */
iNode_info *step_to_target(unsigned char* disk, int fd, char *path, int flag) {
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
                if (match_name(path, file_name, iPathAnchor, iLastChar)) {
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
            return NULL;
        }
        // If we reach the index of the last directory, exit the loop
        if ((flag == 1 && iPathAnchor == iLastDir) || (flag == 0 && iPathAnchor == iLastChar+2)) {
            printf("# Valid path: target directory reached #\n");
            iNode_info *result = malloc(sizeof(iNode_info));
            result->iNode = currInode;
            result->iNode_number = found->inode;
            return result;
        }
        // Update current inode to the new inode for the upcoming iteration
        found = NULL; // Reset found
    }

}


/**
 * Function that find the index of the first free inode in the inode table
 * by traversing through the inode bitmap. Will return -1 if the inode
 * table is compact (no free space for an extra inode)
 */
int find_free_inode(unsigned char *disk) {
    struct ext2_super_block *sb = (struct ext2_super_block *)(disk + EXT2_BLOCK_SIZE);
    // Get inode bitmap
    char *bmi = (char *) get_inode_bitmap(disk);
    
    // The first 11 is reserved, so start from index 11 (the 12^th inode)
    int j = 0;
    int index = -1;
    for (int i = 0; i < sb->s_inodes_count; i++) {
        unsigned c = bmi[i / 8]; // get the corresponding byte
        // If that bit was a 1, inode is used, continue checking.
        if ((c & (1 << j)) == 0 && i > 10) { // > 10 because first 11 are reserved
            index = i;
            break;
        }
        if (++j == 8) {
            j = 0; // increment shift index, if > 8 reset.
        }
    }
    // Set the target inode to be all 0
    memset(get_inode(index, disk), 0, sizeof(struct ext2_inode));
    return index;
}

/**
 * Function that find the index of the first free block in the data blocks
 * by traversing through the block bitmap. Will return -1 if the block
 * table is compact (no free space for an extra inode)
 */
int find_free_block(unsigned char *disk) {
    struct ext2_super_block *sb = (struct ext2_super_block *)(disk + EXT2_BLOCK_SIZE);
    // Get inode bitmap
    char *bmi = (char *) get_block_bitmap(disk);
    
    // The first 11 is reserved, so start from index 11 (the 12^th inode)
    int j = 0;
    int index = -1;
    for (int i = 0; i < sb->s_blocks_count; i++) {
        unsigned c = bmi[i / 8]; // get the corresponding byte
        // If that bit was a 1, inode is used, continue checking.
        if ((c & (1 << j)) == 0) {
            index = i;
            break;
        }
        if (++j == 8) {
            j = 0; // increment shift index, if > 8 reset.
        }
    }
    // Set the target block to be all 0
    memset(get_block(index, disk), 0, EXT2_BLOCK_SIZE);
    return index;
}


/**
 * Print out the block and inode bitmaps for debugging purpose
 */
void printInfo(unsigned char *disk) {
    struct ext2_super_block *sb = (struct ext2_super_block *)(disk + 1024);
    struct   ext2_group_desc *bgd = (struct ext2_group_desc *) (disk + 2048);
    /******************************* Block Bitmap *************************************/
    // Get block bitmap
    char *bm = (char *) (disk + (bgd->bg_block_bitmap * EXT2_BLOCK_SIZE));
    // counter for shift
    printf("block bitmap: ");
    int index = 0;
    for (int i = 0; i < sb->s_blocks_count; i++) {
        unsigned c = bm[i / 8];                     // get the corresponding byte
        printf("%d", (c & (1 << index)) > 0);       // Print the correcponding bit
        if (++index == 8) (index = 0, printf(" ")); // increment shift index, if > 8 reset.
    }
    printf("\n");

    /******************************* Inode Bitmap *************************************/
    // Get inode bitmap
    char *bmi = (char *) (disk + (bgd->bg_inode_bitmap * EXT2_BLOCK_SIZE));
    // counter for shift
    printf("inode bitmap: ");
    int index2 = 0;
    for (int i = 0; i < sb->s_inodes_count; i++) {
        unsigned c = bmi[i / 8];                     // get the corresponding byte
        printf("%d", (c & (1 << index2)) > 0);       // Print the correcponding bit
        // If that bit was a 1, inode is used, store it into the array.
        // Note, this is the index number, NOT the inode number
        // inode number = index number + 1
        if (++index2 == 8) (index2 = 0, printf(" ")); // increment shift index, if > 8 reset.
    }
    printf("\n\n");
}

/**
 * The function take in an inode, and find the first free block inside the inode
 */

    

