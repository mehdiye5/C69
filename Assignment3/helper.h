// This is a header file for the helper.c 
#define BASE_OFFSET 1024 //#
#define BLOCK_OFFSET(block) (BASE_OFFSET + (block-1)*EXT2_BLOCK_SIZE) //#

void * unset_inode_bitmap(int inode_number, unsigned char* disk);
void * unset_block_bitmap(int block_number, unsigned char* disk);
struct ext2_super_block * get_super_block (unsigned char* disk);
char* substring(char* str, size_t begin, size_t end);
int get_root_inode_number(void);
struct ext2_group_desc *get_blocks_group_descriptor(unsigned char* disk);
unsigned char  *get_inode_table(unsigned char* disk);
struct ext2_inode *get_inode(int inode_number, unsigned char* disk);
char* get_file_name(char* directory);
char* get_dir_name(char* directory);
unsigned char *map_image(int fd);
unsigned char * check_inode_block(int block, unsigned char *disk, char * sub_directory, unsigned char  f_type);
int next_index(char * dir);
char * next_sub_dir_name(char *dir);
char *next_dir (char * dir);
unsigned char * find_sub_dir_inode (char * sub_dir_name, struct ext2_inode* dir_inode, unsigned char *disk, unsigned char  f_type);
unsigned char * find_dir_inode(char * directory, unsigned char *disk );
void empty_inode_block(int block_number, unsigned char *disk);
void empty_direct_blocks(int inode_number, unsigned char * disk);
void empty_sigle_indirect_blocks(int inode_number, unsigned char* disk);
int delete_file_inode(char *directory ,unsigned char *disk);
int delete_file_inode(char *directory ,unsigned char *disk);

unsigned char *get_block_bitmap(unsigned char* disk); //#
unsigned char *get_inode_bitmap(unsigned char* disk); //#

unsigned char *get_block(int block_number, unsigned char* disk); //#

int match_name(char *path, char* actual, int start, int last); //#
char** parse_absolute_path(char* inpPath); //#

//#
typedef struct Three_indices {
    int anchor;
    int last_char;
    int last_dir;
} Three_indices;

//#
typedef struct iNode_info {
    struct ext2_inode *iNode;
    int iNode_number;
} iNode_info;

iNode_info *step_to_target(unsigned char* disk, int fd, char *path, int flag); //#
Three_indices generate_position(char *path); //#
int find_free_inode(unsigned char *disk); //#
int find_free_block(unsigned char *disk); //#
int find_spot_for_inode_entry(int inode_number, unsigned char *disk); //#
void update_inode_bitmap (int binary_state, int inode_number, unsigned char *disk); //#
void printInfo(unsigned char *disk); //#
