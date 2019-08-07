/*
This is a helper module where the helper functions will be located, to be used by other programs.
 */

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
 

/**
  * sets the inode number entry in the inode bitmap to 0, meaning free
  * note: integer is 8 bytes so we need to account for that during the bit shift
  */
 void * unset_inode_bitmap(int inode_number, unsigned char* disk) {
     struct ext2_group_desc *blocks = get_blocks_group_descriptor(disk);
    unsigned char * bitmap = (disk + blocks->bg_inode_bitmap * EXT2_BLOCK_SIZE);
    bitmap[(inode_number -1)/8] &= (~(1 << (inode_number - 1) % 8));  
 }

 /**
  * sets the block number entry in the block bitmap to 0, meaning free
  * note: integer is 8 bytes so we need to account for that during the bit shift
  */
 void * unset_block_bitmap(int block_number, unsigned char* disk) {
     struct ext2_group_desc *blocks = get_blocks_group_descriptor(disk);
    unsigned char * bitmap = (disk + blocks->bg_block_bitmap * EXT2_BLOCK_SIZE);
    bitmap[(block_number -1)/8] &= (~(1 << (block_number - 1) % 8));  
 }

/**
  * sets the inode number entry in the inode bitmap to 0, meaning free
  * note: integer is 8 bytes so we need to account for that during the bit shift
  */
 void * set_inode_bitmap(int inode_number, unsigned char* disk) {
    printInfo(disk);
    struct ext2_group_desc *blocks = get_blocks_group_descriptor(disk);
    unsigned char * bitmap = (disk + blocks->bg_inode_bitmap * EXT2_BLOCK_SIZE);
    bitmap[(inode_number -1)/8] |= (1 << ((inode_number - 1) % 8));
    printInfo(disk);
 }

 /**
  * sets the block number entry in the block bitmap to 0, meaning free
  * note: integer is 8 bytes so we need to account for that during the bit shift
  */
 void * set_block_bitmap(int block_number, unsigned char* disk) {
     struct ext2_group_desc *blocks = get_blocks_group_descriptor(disk);
    unsigned char * bitmap = (disk + blocks->bg_block_bitmap * EXT2_BLOCK_SIZE);
    bitmap[block_number >> 3] |= (1 << (block_number - 1) % 8);  
 }


 /**
  * returns pointer to a super block
  */
 struct ext2_super_block * get_super_block (unsigned char* disk) {
     return (struct ext2_super_block *)(disk + EXT2_BLOCK_SIZE);
 }

 /**
  * outputs the substring between the begin index and end index
  */
 char* substring(char* str, size_t begin, size_t end) {

     
  if (str == 0 || strlen(str) == 0 || strlen(str) < begin || strlen(str) < (begin+end))
    return 0;

    char * result = malloc(sizeof(char)*strlen(strndup(str + begin, end)));
    
    result = strndup(str + begin, end);

  return result;
} 

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
 * extract the file name from a directory
 */
char* get_file_name(char* directory) {
    char* copy = strdup(directory);    
    return basename(copy);
}


/**
 * return directory string without the filename
 * i.e /step1/test.txt -> /step1/
 */
char* get_dir_name(char* directory) {
    char* copy = strdup(directory);    
    return strcat(dirname(copy), "/");
}

/**
 * assign pointer to the nmap for the the given image file discriptor
 */
unsigned char *map_image(int fd){
    // note: copied for readimage.c from provided files  from week8&9
    // assign pointer to the nmap for the the given image
    unsigned char * disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(disk == MAP_FAILED) {
	perror("mmap");
	exit(1);
    }
    return disk;
}

/**
 * note concept for this method was taken from http://cs.smith.edu/~nhowe/262/oldlabs/ext2.html#locate_file
 * checks the inode block if it contains  the sub directory
 * if found it returns directory or file entry location
 * otherwise returns 0
 */
unsigned char * check_inode_block(int block, unsigned char *disk, char * sub_directory, unsigned char  f_type) {
    unsigned char *origin = disk + (block * EXT2_BLOCK_SIZE);
    unsigned char * result = NULL;
    

    // Linux kernel sets the inode field of the entry to be deleted to 0, and suitably increments the value of the rec_len field of the previous valid entry.
    // so making sure that inode field is not 0
    if (((struct ext2_dir_entry_2 *) origin)->inode != 0) {
        unsigned char * current_entry_location = origin;

        

        // keep checking the directory entries until we reach the end of the block size
        while (current_entry_location != origin + EXT2_BLOCK_SIZE) {
            // current directory entry
            struct ext2_dir_entry_2* current_directory_entry = (struct ext2_dir_entry_2 *) current_entry_location;

            // make sure that directory entry is a directory
            if (current_directory_entry->file_type == f_type) {
                if (strcmp(current_directory_entry->name, sub_directory) == 0 ) {
                    //inode_number = current_directory_entry->inode;                    
                    result = current_entry_location;
                    break;
                }

            }
            //increment current inode blocks directory positon by directory entry length, which is the location of the next directory entry 
            current_entry_location = current_entry_location + current_directory_entry->rec_len;
        }

    }

    return result;

}


/**
 * returns the next index of the string where the carecter is /
 * i.e home/class -> 4
 */
int next_index(char * dir) {
    int index = 0;   
    
    for (index = 0; index < strlen(dir); index ++) {
        if (dir[index] == '/') {
            break;
        }
    }
    

    return index;
}


/**
 * returns the name of the next sub directory
 * /home/class/ -> home 
 */
char * next_sub_dir_name(char *dir) {
    char * sub_dir_name = NULL;

    int index = next_index(substring(dir,1, strlen(dir) - 1));

    if (index != 0) {
        sub_dir_name = substring(dir, 1, index);
    }
    return sub_dir_name;
}


/**
 * returning dir string without the next sub directory
 * i.e /home/class/rules/ -> /class/rules/
 */
char *next_dir (char * dir) {

    
    char * next = NULL;

    

    int index = next_index(substring(dir,1, strlen(dir) - 1));    

    if (index != 0) {
       //next = substring(dir, index + 1, strlen(dir) - 1);
       next = strndup(dir + index + 1, strlen(dir) - 1);
    }    

    return next;
}

/**
 * given directory inode this function find the inode number for the wanted sub directory
 * if 0 is returned, then inode number couldn't be found
 */
unsigned char * find_sub_dir_inode (char * sub_dir_name, struct ext2_inode* dir_inode, unsigned char *disk, unsigned char  f_type) {
    
    // default inode number
    int inode_number = 0;
    unsigned char * entry_location = NULL;

    // go through every block in the inode
    for (int i = 0; i < 12; i ++){

        // check if the block contains wanted directory
        entry_location = check_inode_block((dir_inode->i_block)[i], disk, sub_dir_name, f_type);

        // inode number was found
        if (entry_location != NULL) {
            break;
        }
        
        
    }

    return entry_location;

}

/**
 * function returns the inode for the given directory that contains the  file
 *
 */
unsigned char * find_dir_inode(char * directory, unsigned char *disk ) {
    struct ext2_inode *inode = get_inode(get_root_inode_number(), disk);

    char * sub_dir_name = NULL;
    char * n_dir = get_dir_name(directory);
    int inode_number = 0;

    unsigned char * entry_location = NULL;

    while (n_dir != NULL)
    {
        sub_dir_name = next_sub_dir_name(n_dir);

        //printf("next sub dir name is: %s \n", n_dir) ;

        //printf("next dir name is: %s \n", sub_dir_name) ;
        
        if (sub_dir_name != NULL) {
            entry_location =  find_sub_dir_inode(sub_dir_name, inode , disk, EXT2_FT_DIR);

            //printf("next inode number is: %d \n", inode_number);
            
            if (entry_location == NULL) {
                perror("Sub directory doesn't exist: ");
            }
            
            inode_number = ((struct ext2_dir_entry_2 *) entry_location)->inode;
            inode = get_inode(inode_number, disk);
            //printf("next dir string is: %s \n", n_dir);

            //printf("next sub dir name: %s \n", next_sub_dir_name(n_dir));
        }
        
        n_dir = next_dir(n_dir);
    }
    return entry_location;
}


/**
 * emties the inode data block, given by the block number
 */
void empty_inode_block(int block_number, unsigned char *disk) {
    get_blocks_group_descriptor(disk)->bg_free_blocks_count ++;
    get_super_block(disk)->s_free_blocks_count ++;
    unset_block_bitmap(block_number, disk);
}


/**
 * function empties first 12 direct blocks
 */
void empty_direct_blocks(int inode_number, unsigned char * disk) {
    
    struct ext2_inode *inode = get_inode(inode_number, disk);

    for (int i = 0; i < 12; i++) {
        empty_inode_block(inode->i_block[i], disk);        
    } 
}

/**
 * function empties single indirect block of inode
 * note: The number of pointers in the indirect blocks is dependent on the block size and size of block pointers.
 */
void empty_sigle_indirect_blocks(int inode_number, unsigned char * disk) {    
    struct ext2_inode *inode = get_inode(inode_number, disk);

    if (inode->i_block[13] != 0) {
        int * singe_indirect_block = (int *)(disk + (inode-> i_block)[13] * EXT2_BLOCK_SIZE);

        for (int i = 0; i < (15*15 + 1); i++) {
            if (singe_indirect_block[i] != 0) {
                empty_inode_block(singe_indirect_block[i], disk);
            }
        }
    }
    
}

/**
 * Delete inode of the given file
 */

int delete_file_inode(char *directory ,unsigned char *disk) {
    unsigned char * parent_entry_location = find_dir_inode(directory, disk);
    
    int parent_dir_inode_number = ((struct ext2_dir_entry_2 *) parent_entry_location)->inode;

    char * filename = get_file_name(directory);

    struct ext2_inode * parent_inode = get_inode(parent_dir_inode_number, disk);

    unsigned char * file_entry_location =  find_sub_dir_inode(filename, parent_inode , disk, EXT2_FT_REG_FILE);

    int file_inode_number = ((struct ext2_dir_entry_2 *) file_entry_location)->inode;

    struct ext2_inode * file_inode = get_inode(file_inode_number, disk);    


    // decrement the number of links i_links value of the file_inode
    // i_links value illustrate the number of hardlinks
    // if i_links value is 0 then we can free inode blocks
    file_inode->i_links_count --;

    if (file_inode->i_links_count == 0) {
        // free file inode blocks
        empty_direct_blocks(file_inode_number, disk);
        empty_sigle_indirect_blocks(file_inode_number, disk);

        get_blocks_group_descriptor(disk)->bg_free_inodes_count ++;
        get_super_block(disk)->s_free_inodes_count ++;
        unset_inode_bitmap(file_inode_number, disk);        
    }

    struct ext2_dir_entry_2* current_directory_entry = ((struct ext2_dir_entry_2 *) file_entry_location);
    unsigned char * previous_block_location = (file_entry_location - current_directory_entry->rec_len);
    struct ext2_dir_entry_2* previous_directory_entry = (struct ext2_dir_entry_2 *) previous_block_location;
    previous_directory_entry->rec_len += current_directory_entry->rec_len;
}


/*--------------------------- functions added by Robert later below ---------------------------*/

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
 function return the block for the given block index in the block bitmap
*/
unsigned char *get_block(int block_number, unsigned char* disk) {
    return disk + (block_number-1) * EXT2_BLOCK_SIZE;
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
    // Cannot generate under the case the input is the root directory
    if (iLastChar == -1) {
        Three_indices special_case = {
            .anchor = -1,
            .last_char = -1,
            .last_dir = -1};
        return special_case;
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

    if (iPathAnchor == -1) {
        if (flag) {
            return NULL;
        }
        else {
            iNode_info *special_case = malloc(sizeof(iNode_info));
            special_case->iNode = get_inode(2, disk);
            special_case->iNode_number = 2;
            return special_case;
        }
    }

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
                    // printf("# found file %s, inode %u #\n", file_name, entry->inode);
                    // printf("------------ next round ------------\n");
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
            
            // printf("size: %d, rec_len: %d\n", size, currInode->i_size);
        }
        // If we do not find any inode that: it is a directory inode and it match the name, then complain
        if (found == NULL) {
            return NULL;
        }
        // If we reach the index of the last directory, exit the loop
        if ((flag == 1 && iPathAnchor == iLastDir) || (flag == 0 && iPathAnchor == iLastChar+2)) {
            //printf("# Valid path: target directory reached #\n");
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
 * Check if the block with number=block_number is free
 */
int is_block_free(int block_number, unsigned char *disk) {
    struct ext2_super_block *sb = (struct ext2_super_block *)(disk + EXT2_BLOCK_SIZE);
    // Get inode bitmap
    char *bmi = (char *) get_block_bitmap(disk);
    
    // The first 11 is reserved, so start from index 11 (the 12^th inode)
    int j = 0;
    int index = -1;
    for (int i = 0; i < sb->s_blocks_count; i++) {
        // Reaching the input block number
        if (i == block_number - 1) {
            unsigned c = bmi[i / 8]; // get the corresponding byte
            // If the byte is 0, the block is free
            return ((c & (1 << j)) == 0);
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
 * Find the block number of first free block that belongs to an inode's i_block field
 */
int find_spot_for_inode_entry(int inode_number, unsigned char *disk) {
    struct ext2_inode *inode = get_inode(inode_number, disk);
    
    for (int i = 0; i < 12; i ++) {
        int curr_block_number = inode->i_block[i];
        if (curr_block_number == 0) {
            int new_allocated = find_free_block(disk);
            if (new_allocated == -1) {
                return -1;
            }
            inode->i_block[i] = new_allocated;
            // TODO: flip the bit
            set_block_bitmap(new_allocated + 1, disk);
            return new_allocated + 1;
        }
        if (is_block_free(curr_block_number, disk)) {
            // TODO: flip the bit
            set_block_bitmap(curr_block_number, disk);
            return curr_block_number;
        }
    }
    // Direct blocks not free
    int *indirect_blk = (int *)(disk + (inode->i_block)[12] * EXT2_BLOCK_SIZE);
    for (int i = 0; i < 225; i ++) {
        int curr_block_number = indirect_blk[i];
        if (curr_block_number == 0) {
            int new_allocated = find_free_block(disk);
            if (new_allocated == -1) {
                return -1;
            }
            indirect_blk[i] = new_allocated;
            // TODO: flip the bit
            set_block_bitmap(new_allocated + 1, disk);
            return new_allocated + 1;
        }
        if (is_block_free(curr_block_number, disk)) {
            // TODO: flip the bit
            set_block_bitmap(curr_block_number, disk);
            return curr_block_number;
        }
    }
    // No free block under direct and first-level indirect
    return -1;
}

void update_inode_bitmap (int binary_state, int inode_number, unsigned char *disk) {
    // struct ext2_super_block *sb = (struct ext2_super_block *)(disk + EXT2_BLOCK_SIZE);
    // // Get inode bitmap
    // char *bmi = (char *) get_inode_bitmap(disk);
    // if (binary_state == 1) {
    //     bmi |= 1UL << (inode_number-1);
    //     return;
    // }
    // if (binary_state = 0) {
    //     bmi &= ~(1UL << (inode_number-1));
    // }
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
