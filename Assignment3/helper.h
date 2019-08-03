// This is a header file for the helper.c 

int get_root_inode_number(void);
struct ext2_group_desc *get_blocks_group_descriptor(unsigned char* disk);
unsigned char *get_block_bitmap(unsigned char* disk); // added in Robert's branch
unsigned char *get_inode_bitmap(unsigned char* disk); // added in Robert's branch
unsigned char  *get_inode_table(unsigned char* disk);
struct ext2_inode *get_inode(int inode_number, unsigned char* disk);
int match_name(char *path, char* actual, int start, int last); // added in Robert's branch
char** parse_absolute_path(char* inpPath); // added in Robert's branch

