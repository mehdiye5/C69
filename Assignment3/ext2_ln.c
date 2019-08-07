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
#include <stdbool.h>

/*
This program takes three command line arguments. The first is the name of an ext2 formatted
virtual disk. The other two are absolute paths on your ext2 formatted disk. The program should work
like ln, creating a link from the first specified file to the second specified path. If the source file does not
exist (ENOENT), if the link name already exists (EEXIST), or if either location refers to a directory
(EISDIR), then your program should return the appropriate error. Note that this version of ln only works
with files. Additionally, this command may take a "-s" flag, after the disk image argument. When this flag
is used, your program must create a symlink instead (other arguments remain the same). If in doubt
about correct operation of links, use the ext2 specs and ask on the discussion board.
 */

//nmap pointer
unsigned char *disk;

int main(int argc, char **argv) {
    int is_flag = 0; //0 for false, 1 for true

    //incorrect number of args
    if (!(argc == 4) || (argc == 5)){
        fprintf(stderr, "Usage: ext2_ln <image file name>\n");
        exit(1);
    }

    if (argc == 5) {
        is_flag = 1;
    }
    //open image file
    int fd = open(argv[1], O_RDWR);

    disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(disk == MAP_FAILED) {
	    perror("mmap");
	    exit(1);
    }

    struct ext2_group_desc *bgd = (struct ext2_group_desc *)(disk + (EXT2_BLOCK_SIZE *  2));
    struct ext2_inode *inode_table = (struct ext2_inode *)(disk + (EXT2_BLOCK_SIZE * bgd->bg_inode_table));

    //find the inode to link to 
    char *link_to = argv[2];
    char file = get_file_name(link_to);
    int inode_tolink = find_inode(link_to);
    char *this_link = argv[3];

    if (is_flag == 0){
        char dir = get_dir_name(link_to);
        int inode_num = ((struct ext2_dir_entry_2 *) dir) -> inode;
        
        //this is where it will be placed into
        struct ext2_inode *dir_inode = get_inode(inode_num, disk);
        //this is where it currently is
        struct ext2_inode *currently = get_inode(inode_tolink, disk);
        //insert into the directory
        //insert(file, dir_inode, currently)

        // check if there is a free node 
        // code also from ext2_cp
        // Index of a free inode
        int iInode = find_free_inode(disk);
        if (iInode == -1) {
            fprintf(stderr, "Disk compact.");
            exit(1);
        } else { printf("Inode index found: %d\n", iInode); }

        // Pointer to the free inode
        struct ext2_inode *newInode = get_inode(iInode + 1, disk);
        newInode->i_mode = EXT2_S_IFREG;

        // Directory entry for the free inode
        int newFileEtryNum = find_spot_for_inode_entry(inode_num, disk);
        struct ext2_dir_entry_2 *newFileEtry = (struct ext2_dir_entry_2 *)get_block(newFileEtryNum, disk);

        // Set relevant attributes
        newFileEtry->file_type = EXT2_FT_REG_FILE;
        newFileEtry->inode = iInode + 1;
        int new_name_len = strlen(file);
        newFileEtry->rec_len = new_name_len + 8; // TODO: check if the rec_len here is fine. Entry length is the length of the new directory name
        newFileEtry->name_len = new_name_len;
        memcpy(newFileEtry->name, file, new_name_len);
        return currently;
    } else if (is_flag == 1) {

    }
    

    return 0;
}

int find_inode(char *path) {
    //root inode
    int curr = EXT2_ROOT_INO;
    struct ext2_inode *inode = get_inode(curr, disk);
    //split path by '/'
    char *dirs = strtok(path, "/");

    // loop through dirs
    while(dirs != NULL) {
       
        if (inode->i_mode & EXT2_S_IFDIR) {
            char entry_location = find_dir_inode(dirs, disk);
            struct ext2_dir_entry_2 *d = ((struct ext2_dir_entry_2 *) entry_location);
            if (d == NULL) { //source file does not exist
                return -ENOENT;
            }

            //inode number
            curr = d->inode;
            //the inode
            inode = get_inode(curr, disk);
            //next dir
            dirs = strtok(NULL, "/");
        } else {
            return -ENOTDIR;
        }

    }
    return curr;
}