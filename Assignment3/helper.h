// This is a header file for the helper.c 

// "//#" is the method later added by Robert. Please let me know if find any error
#define BASE_OFFSET 1024 //#
#define BLOCK_OFFSET(block) (BASE_OFFSET + (block-1)*EXT2_BLOCK_SIZE) //#


int get_root_inode_number(void);
struct ext2_group_desc *get_blocks_group_descriptor(unsigned char* disk);
unsigned char *get_block_bitmap(unsigned char* disk); //#
unsigned char *get_inode_bitmap(unsigned char* disk); //#
unsigned char  *get_inode_table(unsigned char* disk);
struct ext2_inode *get_inode(int inode_number, unsigned char* disk);
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

