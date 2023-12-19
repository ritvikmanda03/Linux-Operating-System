#include "x86_desc.h"
#include "keyboard.h"

//size of a block in the file system in bytes
#define BLOCK_SIZE 4096

//function to initialize global pointers including address, boot block, inodes, and data blocks
void initialize_pointers(uint32_t module_address);

//struct to hold a directory entry
typedef union dentry {
    uint8_t val[64];
    struct {
        char file_name[32];             //32 character file name
        uint32_t file_type;             //4 byte file type
        uint32_t inode_n;               //4 byte inode index
        char reserved[24];              //reserved bytes
    } __attribute__ ((packed));
} dentry_t;

//struct to hold a boot block
typedef union boot_block {
    uint8_t val[BLOCK_SIZE];
    struct {
        uint32_t dir_entries_n;             //4 byte # of directory entries
        uint32_t total_i;                  //4 byte # of inode blocks
        uint32_t data_blocks_n;           //4 byte # of data blocks
        char reserved[52];               //reserved bytes
        dentry_t dir_entries[64 - 1];   //63 64 byte directory entries
    } __attribute__ ((packed));
} boot_block_t;

//struct to hold an inode
typedef struct inode {
    uint32_t length;                     //4 byte length of the file in bytes
    uint32_t data_blocks[1024 - 1];     //1023 4 byte data block indices
} inode_t;

//struct to hold a data block
typedef struct data_block {
    uint8_t data_array[BLOCK_SIZE];     //4kB data block array
} data_block_t;

/* jumptable struct for open read write functions for each file type */
typedef struct table_pointer {
    /* make sure to typecast the arguments as that specific function type */
    int32_t (*open) (const uint8_t* filename);
    int32_t (*close) (int32_t fd);
    int32_t (*read) (int32_t fd, void* buf, int32_t nbytes);
    int32_t (*write) (int32_t fd, const void* buf, int32_t nbytes);   
} table_pointer_t;

//struct to hold a file descriptor array entry
typedef struct file_entry {
    table_pointer_t table_pointer;          //4 byte file operations table pointer
    uint32_t inode_idx;               //4 byte inode index
    uint32_t position;               //4 byte file position
    uint32_t flags;                 //4 byte flags
} file_entry_t;

//create global pointers for accessing start emmory address of file system locations
boot_block_t *boot_block;
inode_t *inodes;
data_block_t *data_blocks;
file_entry_t file_array[8];

//define three file system routines to read directory entries or segments of the file
//these routines are to be used by the open and read system calls
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

//define four relevant system calls for files
//calls from the cpu can be routed here for files
int32_t open_file (const uint8_t* filename);
int32_t close_file (int32_t fd);
int32_t read_file (int32_t fd, void* buf, int32_t nbytes);
int32_t write_file (int32_t fd, const void* buf, int32_t nbytes);

//define four relevant system calls for directories
//calls from the cpu can be routed here for directories
int32_t open_dir (const uint8_t* filename);
int32_t close_dir (int32_t fd);
int32_t read_dir (int32_t fd, void* buf, int32_t nbytes);
int32_t write_dir (int32_t fd, const void* buf, int32_t nbytes);
