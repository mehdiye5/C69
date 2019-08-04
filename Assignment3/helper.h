// This is a header file for the helper.c 

int get_root_inode_number(void);
struct ext2_group_desc *get_blocks_group_descriptor(unsigned char* disk);
unsigned char  *get_inode_table(unsigned char* disk);
struct ext2_inode *get_inode(int inode_number, unsigned char* disk);

