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
 

/**
  * sets the inode number entry in the inode bitmap to 0, meaning free
  * note: integer is 8 bytes so we need to account for that during the bit shift
  */
 void * unset_inode_bitmap(int inode_number, unsigned char* disk) {
     struct ext2_group_desc *blocks = get_blocks_group_descriptor(disk);
    unsigned char * bitmap = (disk + blocks->bg_inode_bitmap);
    bitmap[(inode_number -1)/8] &= (~(1 << (inode_number - 1) % 8));  
 }

 /**
  * sets the block number entry in the block bitmap to 0, meaning free
  * note: integer is 8 bytes so we need to account for that during the bit shift
  */
 void * unset_block_bitmap(int block_number, unsigned char* disk) {
     struct ext2_group_desc *blocks = get_blocks_group_descriptor(disk);
    unsigned char * bitmap = (disk + blocks->bg_block_bitmap);
    bitmap[(block_number -1)/8] &= (~(1 << (block_number - 1) % 8));  
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
    for (int i = 0; i < dir_inode->i_blocks; i ++){

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