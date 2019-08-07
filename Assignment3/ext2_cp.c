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
    if (argc != 4) {
        fprintf(stderr, "Usage: cp <image file name> <path to a file on OS> <absolute path on ext2 disk>\n");
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
    // Pointer to the super block
   struct ext2_super_block *sb = (struct ext2_super_block *)(disk + EXT2_BLOCK_SIZE);
   struct ext2_group_desc *bgd = (struct ext2_group_desc *) (disk + 2*EXT2_BLOCK_SIZE);

    // Open the input file
    FILE *fp = fopen(argv[2], "r");
    if(fp == NULL) {
		fprintf(stderr, "File path on OS is invalid\n");
		exit(1);
	}

    // // Relevant indices
    // Three_indices exIndices = generate_position(argv[3]);
    // //int iPathAnchor = indices.anchor;
    // int iExLastChar = exIndices.last_char;
    // int iExLastDir = exIndices.last_dir;

    // Step the directory where the file will be copies
    iNode_info *last_info = step_to_target(disk, fd, argv[3], 0);
    if (last_info == NULL) {
            printf("! Invalid path !\n");
            exit(1);
    }
    struct ext2_inode *last_inode = last_info->iNode;
    int last_inode_number = last_info->iNode_number;
    printf("%d\n", last_inode_number);
    fflush(stdout);

    /* --------------------- Check if the directory creating exists --------------------------*/
    // Check one more round to see if the file name we are copying exists in the current directory
    Three_indices osIndices = generate_position(argv[2]);
    //int iPathAnchor = indices.anchor;
    int iOsLastChar = osIndices.last_char;
    int iOsLastDir = osIndices.last_dir;

    struct ext2_dir_entry_2 *entry;
    unsigned int size;
    unsigned char block[EXT2_BLOCK_SIZE];
    // Read the first block of the current inode
    lseek(fd, BLOCK_OFFSET(last_inode->i_block[0]), SEEK_SET);
    read(fd, block, EXT2_BLOCK_SIZE);
    size = 0; // Bytes read so far
    entry = (struct ext2_dir_entry_2 *) block; // entry is initialized to the first block of the current inode
    while(size < last_inode->i_size) {
        // Get the file name of the ext2_dir_entry_2
        char file_name[EXT2_NAME_LEN+1];
        memcpy(file_name, entry->name, entry->name_len);
        file_name[entry->name_len] = 0; // null char at the end
        // Directory with the desired name matched
        if (match_name(argv[2], file_name, iOsLastDir, iOsLastChar)) {
            fprintf(stderr, "File \"%s\" trying to copy already exists\n", file_name);
            exit(1);
        }
        // Else update relavent index to keep checking the rest of the files
        size += entry->rec_len; // update size we have read so far
        entry = (void*) entry + entry->rec_len; // move to the next entry
        
        printf("size: %d, rec_len: %d\n", size, last_inode->i_size);
    }
    // Function does not exit in the above loop --> path okay
    printf("# Path check passed: input path okay #\n");

    printf("right before loading data");
    printInfo(disk);

    /* ----------------- Load the file information into the inode blocks --------------------------*/
    // Index of a free inode
    int iInode = find_free_inode(disk);
    if (iInode == -1) {
        fprintf(stderr, "Disk compact.");
        exit(1);
    } else { printf("Inode index found: %d\n", iInode); }
    // Pointer to the free inode
    struct ext2_inode *newInode = get_inode(iInode + 1, disk);
    newInode->i_mode = EXT2_S_IFREG;
    set_inode_bitmap(iInode+1, disk);

    printf("inode found at index %d\n", iInode);
    printInfo(disk);
    
    // Directory entry for the free inode
    int newFileEtryNum = find_spot_for_inode_entry(last_inode_number, disk);
    struct ext2_dir_entry_2 *newFileEtry = (struct ext2_dir_entry_2 *)get_block(newFileEtryNum, disk); // TODO: implement in the helper function
    // Set relevant attributes
    newFileEtry->file_type = EXT2_FT_REG_FILE;
    newFileEtry->inode = iInode + 1;
    int new_name_len = iOsLastChar - iOsLastDir + 1;
    newFileEtry->rec_len = new_name_len + 8; // TODO: check if the rec_len here is fine. Entry length is the length of the new directory name
    newFileEtry->name_len = new_name_len;
    memcpy(newFileEtry->name, argv[2]+iOsLastDir, new_name_len);
    set_block_bitmap(newFileEtryNum, disk);
    
    printf("block found at number %d\n", newFileEtryNum);
    printInfo(disk);

    // Total number of blocks needed for the file
    fseek(fp, 0, SEEK_END);
	int blocks_needed = ftell(fp) / EXT2_BLOCK_SIZE + 1; // Round up the result
    rewind(fp);
    
    // Fill the first 12 direct blocks
    for (int i = 0; i < 12; i ++) {
        // Stop loading when no more blocks are needed
        if (blocks_needed == 0) {
            printf("Direct blocks are good enough");
            break;
        }
        // Find a free block
        int iBlock = find_free_block(disk);
        if (iBlock == -1) {
            fprintf(stderr, "Disk compact.");
            exit(1);
        }
        // TODO: figure out what type is stored in i_block
        (newInode->i_block)[i] = iBlock;
        struct ext2_dir_entry_2*newInodeBlk = (struct ext2_dir_entry_2 *)get_block(iBlock+1, disk);
        // Load the file data to the block
        fread(newInodeBlk, EXT2_BLOCK_SIZE, 1, fp);
        // Decrement the remaining required blocks count
        blocks_needed--;
    }

    // Single redirection if direct blocks are not good enough
    if (blocks_needed != 0) {
        int *indirect_blk = (int *)(disk + (newInode->i_block)[12] * EXT2_BLOCK_SIZE);
        // Get pointer to the indirect block
        for (int i = 0; i < 225; i ++) {
            // Stop loading when no more blocks are needed
            if (blocks_needed == 0) {
                printf("Direct blocks are good enough");
                break;
            }
            // Find a free block
            int iBlock = find_free_block(disk);
            if (iBlock == -1) {
                fprintf(stderr, "Disk compact.");
                exit(1);
            }
            struct ext2_dir_entry_2*newInodeBlk = (struct ext2_dir_entry_2 *)get_block(iBlock+1, disk);
            (newInode->i_block)[i] = iBlock;
            // Load the file data to the block
            fread(newInodeBlk, EXT2_BLOCK_SIZE, 1, fp);
            // Set the indirect block's block number
            indirect_blk[i] = iBlock;
            // Decrement the remaining required blocks count
            blocks_needed--;
        }
    }

    if (blocks_needed != 0) {
        fprintf(stderr, "File size too big");
        exit(1);
    }

    bgd->bg_used_dirs_count ++;
    bgd->bg_free_inodes_count --;
    bgd->bg_free_blocks_count --;
    sb->s_free_inodes_count --;
    sb->s_free_blocks_count --;

    printf("cp done");

    return 0;

}
