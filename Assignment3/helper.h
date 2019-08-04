<<<<<<< HEAD
// This is a header file for the helper.c 

int get_root_inode_number(void);
struct ext2_group_desc *get_blocks_group_descriptor(unsigned char* disk);
unsigned char  *get_inode_table(unsigned char* disk);
struct ext2_inode *get_inode(int inode_number, unsigned char* disk);

=======
// This is a header file for the helper.c 

int get_root_inode_number(void);
struct ext2_group_desc *get_blocks_group_descriptor(unsigned char* disk);
unsigned char  *get_inode_table(unsigned char* disk);
struct ext2_inode *get_inode(int inode_number, unsigned char* disk);

>>>>>>> c1b1fbcb5d4c2211b5f50a72cbceef3358b2f9eb
