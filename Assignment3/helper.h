// This is a header file for the helper.c 

char* substring(char* str, size_t begin, size_t end);
int get_root_inode_number(void);
struct ext2_group_desc *get_blocks_group_descriptor(unsigned char* disk);
unsigned char  *get_inode_table(unsigned char* disk);
struct ext2_inode *get_inode(int inode_number, unsigned char* disk);
char* get_file_name(char* directory);
char* get_dir_name(char* directory);
unsigned char *map_image(int fd);
int check_inode_block(int block, unsigned char *disk, char * sub_directory, unsigned char  f_type);
int next_index(char * dir);
char * next_sub_dir_name(char *dir);
char *next_dir (char * dir);
int find_sub_dir_inode (char * sub_dir_name, struct ext2_inode* dir_inode, unsigned char *disk, unsigned char  f_type);
int find_dir_inode(char * directory, unsigned char *disk );
int delete_file_inode(char *directory ,unsigned char *disk);