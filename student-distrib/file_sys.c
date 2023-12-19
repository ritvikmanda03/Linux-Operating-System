#include "file_sys.h"
#include "lib.h"

/* global variable to hold the module address */
uint32_t global_address = 0;

/* 
 *   initialize_pointers(uint32_t module_address)
 *   DESCRIPTION: initializes the global stats of the file system through type casting and initializes the file array
 *                for the file descriptors. 
 *   INPUTS: module starting address of the file system
 *   OUTPUTS: NONE
 *   SIDE EFFECTS: initializes the global_address, boot_block, inodes, and data_blocks variables for later use 
 */
void initialize_pointers(uint32_t module_address){
    /* initialize a global variable to hold the starting address of the file system */
    global_address = module_address;
    /* type casting the boot block, inodes, and data blocks to our defined structs */
    boot_block = (boot_block_t * )(module_address);
    inodes = (inode_t * )(module_address + BLOCK_SIZE);
    data_blocks = (data_block_t * )(module_address + BLOCK_SIZE + (boot_block->total_i * BLOCK_SIZE));

    /* initialize stdin and stdout in file array to be in use */
    file_array[0].flags = 1;
    file_array[1].flags = 1;

    /* initialize the rest of the file array to be not in use */
    int i;
    for(i = 2; i < 8; i++){
        file_array[i].flags = 0;
    }
}

/* 
 *   read_dentry_by_name (const uint8_t* fname, dentry_t* dentry)
 *   DESCRIPTION: finds if a given file_name is valid and then calls the read dentry by index func
 *   INPUTS: file_name and a directory entry
 *   OUTPUTS: return value if it succeed or failed
 *   SIDE EFFECTS: initializes the dentry 
 */
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry){
    if(fname == NULL){return -1;}
    /* check to see if the given filename is more than what can be stored */
    if(strlen((const char *)fname) > 32){ return -1;}
    uint32_t i;
    /* checks to see if the given file name is valid or not */
    for(i = 0; i < boot_block->dir_entries_n; i++){
        /* if the file name is found and, then we can read the directory entry by index */
        if(strncmp((const char *)fname, boot_block->dir_entries[i].file_name, strlen((const char *)fname)) == 0){
            if((strlen((const char *)fname) == strlen(boot_block->dir_entries[i].file_name)) || ((strlen((const char *)fname) == 32) && (strlen(boot_block->dir_entries[i].file_name) == 33))){
                return read_dentry_by_index(i, dentry);
            }
        }
    }
    /* return -1 if failed */
    return -1;
}

/* 
 *   read_dentry_by_index (uint32_t index, dentry_t* dentry)
 *   DESCRIPTION: initializes the given dentry pointer given the index for a file
 *   INPUTS: index of the file_name and a directory entry
 *   OUTPUTS: return value if it succeed or failed
 *   SIDE EFFECTS: initializes the dentry 
 */
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry){
    /* check to see if the index is valid or not */
    if(index < 0 || index > boot_block->dir_entries_n){
        return -1;
    }
    if (dentry == NULL) {
        return -1;
    }
    /* initialize the dentry by pointing it to the actual location in the directory entries */
    *dentry = boot_block->dir_entries[index]; 
    return 0;
}

/* 
 *   read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
 *   DESCRIPTION: Parses through the file and iterates through the blocks and using the length given,
 *                determines when and how much data should be stored in the buffer through checks and memcpy statements
 *   INPUTS: index of the file_name, offset into the file, buffer to store read contents, and the length of how much to read
 *   OUTPUTS: # of bytes read
 *   SIDE EFFECTS: Buffer pointer stores contents of what has just been read 
 */
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
    /* determine the current inode */
    inode_t current_node = inodes[inode];
    /* determine the actual size of the file */
    uint32_t size_of_file = current_node.length;

    /*if the offset is past the size of the file, cannot complete read */
    if(offset > size_of_file){ return -1; }      
    /*if the offset is at the last point in the file, nothing to read but no errors */
    if(offset == size_of_file){ return 0; }     

    /* deterime which block the data is stored in and the offset into that specific block */
    uint32_t block_idx = offset / BLOCK_SIZE;
    uint32_t offset_into_block = offset % BLOCK_SIZE;

    /* determine the actual block index by iterating through the current node data structure */
    uint32_t real_idx = current_node.data_blocks[block_idx]; 

    /* initialize variables */
    uint32_t chars_read, can_read_in_this_block, dataRemaining, newOffset;
    chars_read = 0;
    /* determine how much can be read in a certain block */
    can_read_in_this_block = BLOCK_SIZE - offset_into_block;
    /* determine the total data left to read */
    dataRemaining = size_of_file - offset;
    newOffset = offset_into_block;

    /* while loops to determine when we are done reading for a given call */
    while (chars_read < length && (chars_read + offset) < size_of_file){
        
        /* make sure the total remaining data is less than the length we need to read by */
        if (dataRemaining <= length){

            /* check if the data in the current block is the data in the last data block */
            if (dataRemaining < can_read_in_this_block) {
                /* if it is, then we copy the data into the buffer */
                memcpy(buf+chars_read, (unsigned char *)&data_blocks[real_idx] + newOffset, dataRemaining);
                /* increment the number of chas read and decrement how much total data is left to read*/
                chars_read += dataRemaining;
                dataRemaining -= dataRemaining;
                newOffset += dataRemaining;
            }
            else {
                /* if it is not, just copy the rest of what is in the block to the buffer */
                memcpy(buf+chars_read, (unsigned char *)&data_blocks[real_idx] + newOffset, can_read_in_this_block);
                /* increment the number of chas read and decrement how much total data is left to read*/
                chars_read += can_read_in_this_block;
                dataRemaining -= can_read_in_this_block;
                newOffset += can_read_in_this_block;
            }

        /* in the pcase that data_remaining is greater than length, an edge case*/
        } else {

            /* check if the length is less than what we can read in a block */
            if (length - chars_read < can_read_in_this_block) {
                memcpy(buf+chars_read, (unsigned char *)&data_blocks[real_idx] + newOffset, length - chars_read);
                /* increment the number of chas read and decrement how much total data is left to read*/
                chars_read += (length - chars_read);
                dataRemaining -= (length - chars_read);
                newOffset += (length - chars_read);
            }
            else { /* if the length is not less than what we can read in the block, then */
                memcpy(buf+chars_read, (unsigned char *)&data_blocks[real_idx] + newOffset, can_read_in_this_block);
                /* increment the number of chas read and decrement how much total data is left to read*/
                chars_read += can_read_in_this_block;
                dataRemaining -= can_read_in_this_block;
                newOffset += can_read_in_this_block;
            }

        }
        /* reinitialize the new offset */
        if(newOffset == BLOCK_SIZE){
            newOffset = 0;
            can_read_in_this_block = BLOCK_SIZE;
            /* determine the actual next data block */
            real_idx = current_node.data_blocks[++block_idx];
        }
    }

    /* return the # of chars read */
    return chars_read;
}

/* 
 *   open_file (const uint8_t* filename)
 *   DESCRIPTION: detemines if a file can be opened or not and if it can, returns the index of the  
 *                newly allocated spot in the file array, which is the file descripter
 *   INPUTS: file name
 *   OUTPUTS: returns the file descripter
 *   SIDE EFFECTS: NONE
 */
int32_t open_file (const uint8_t* filename){
    /* check to see the next available slot to open a file */
    int32_t file_desc = 0;
    while(file_array[file_desc].flags == 1) { 
        file_desc++;  
        if (file_desc >= 8) {
            /* return -1 on fail if we got past the indexes of the array */
            return -1;
        } 
    }
    /* create a dentry and check to to see if we can find the file_name*/
    dentry_t temp_dentry;
    int rv = read_dentry_by_name(filename, &temp_dentry);
    if(rv == -1){ return rv;}
    /* set up the file descriptor */
    file_array[file_desc].inode_idx = temp_dentry.inode_n;
    /* file is at the start */
    file_array[file_desc].position = 0;
    /* file is now in use */
    file_array[file_desc].flags = 1;

    /* return the file descriptor */
    return file_desc;
}

/* 
 *   close_file (int32_t fd)
 *   DESCRIPTION: detemines if a file can be closed or not given the file descriptor. Once it is able to,
 *                it sets the file back into not in use for the future.
 *   INPUTS: file descripter
 *   OUTPUTS: return value if it succeed or failed
 *   SIDE EFFECTS: NONE
 */
int32_t close_file (int32_t fd){
    /* check to see if the file is not stdin or stdout */
    if(fd > 1 && fd < 8){
        if(file_array[fd].flags == 0){
        return -1;
        }
        /* if it is not, then it is a regular file and we put it not into use */
        file_array[fd].flags = 0;
        return 0;
    }
    /* return -1 on fail */
    return -1;
}

/* 
 *   read_file (int32_t fd, void* buf, int32_t nbytes)
 *   DESCRIPTION: Calls the read data function to populate the buffer given the file index and the offset into the file.
 *                It updates the current position in the file after read is called.
 *   INPUTS: file descripter, the buffer, and the number of bytes to read
 *   OUTPUTS: return the number of bytes read
 *   SIDE EFFECTS: NONE
 */
int32_t read_file (int32_t fd, void* buf, int32_t nbytes){
    /* check to see if fd is 0 or not */
    if (fd == 0) {
        return terminal_read(fd, buf, nbytes);
    } else if (fd == 1){
        return -1;
    }
    /* should only be activated for stdin and not stdout --> fd = 1 */

    /* call the read_data function */
    int32_t rv = read_data(file_array[fd].inode_idx, file_array[fd].position, buf, nbytes);
    /* update the current position in the file */
    file_array[fd].position += rv;
    /* return # of bytes read */
    return rv;
}

/* 
 *   write_file (int32_t fd, const void* buf, int32_t nbytes)
 *   DESCRIPTION: Writes to the read_only file system, which is not a valid operation
 *   INPUTS: file descripter, the buffer, and the number of bytes to write
 *   OUTPUTS: return the number of bytes written
 *   SIDE EFFECTS: NONE
 */
int32_t write_file (int32_t fd, const void* buf, int32_t nbytes){
    /* we should never write in a read-only file system */
    /* check to see if fd is stdout for write */
    if (fd == 1) {
        return terminal_write(fd, buf, nbytes);
    } 

    /* return -1 on failure */
    return -1;
}

/* 
 *   open_dir (const uint8_t* filename)
 *   DESCRIPTION: detemines if a directory can be opened or not and if it can, returns the index of the  
 *                newly allocated spot in the file array, which is the file descripter
 *   INPUTS: directory name
 *   OUTPUTS: returns the file descripter
 *   SIDE EFFECTS: NONE
 */
int32_t open_dir (const uint8_t* filename){
    /* check to see the next available slot to open a directory */
    int32_t file_desc = 0;
    while(file_array[file_desc].flags == 1) { 
        file_desc++;  
        if (file_desc >= 8) {
            /* return -1 on fail if we got past the indexes of the array */
            return -1;
        } 
    }
    /* create a dentry and check to to see if we can find the directory name*/
    dentry_t temp_dentry;
    read_dentry_by_name(filename, &temp_dentry);
    if (temp_dentry.file_type != 1){return -1;}
     /* set up the file descriptor */
    file_array[file_desc].inode_idx = temp_dentry.inode_n;
    /* setting up positon as in index for each file in the directory */
    file_array[file_desc].position = 0;
    /* directory name is in use */
    file_array[file_desc].flags = 1;

    // return the file descriptor 
    return file_desc;
}

/* 
 *   close_dir (int32_t fd)
 *   DESCRIPTION: detemines if a directory can be closed or not given the file descriptor. Once it is able to,
 *                it sets the directory back into not in use for the future.
 *   INPUTS: file descripter
 *   OUTPUTS: return value if it succeed or failed
 *   SIDE EFFECTS: NONE
 */
int32_t close_dir (int32_t fd){
    /* check to see if the directory is not stdin or stdout */
    if(fd > 1 && fd < 8){
        if(file_array[fd].flags == 0){
        return -1;
        }
        /* if it is not, then it is a regular directory and we put it not into use */
        file_array[fd].flags = 0;
        return 0;
    }
    /* return -1 on fail */
    return -1;
}

/* 
 *   read_dir (int32_t fd, void* buf, int32_t nbytes)
 *   DESCRIPTION: Calls the read dentry by index function to populate a temp directory entry to obtain the information about
 *                a certain file, given the position which is the index to step into the directory entries in the boot block.
 *                The temp dentry is then used to populate the buffer.
 *   INPUTS: file descripter, the buffer, and the number of bytes to read
 *   OUTPUTS: return the number of files read
 *   SIDE EFFECTS: NONE
 */
int32_t read_dir (int32_t fd, void* buf, int32_t nbytes){
    /* check to see if the current position/file index is valid */
    if (file_array[fd].position >= boot_block->dir_entries_n){
        return 0;
    }
    /* create a temp dentry so we can read the filename by index */
    dentry_t temp_dentry;
    read_dentry_by_index(file_array[fd].position, &temp_dentry);
    /* copy the file name into the buffer*/
    strncpy(buf, temp_dentry.file_name, 32);
    /* go to the next file in the directory entries */
    file_array[fd].position++;
    /* return 1 for the number of files being read, once per read*/
    return file_array[fd].position ;
}

/* 
 *   write_dir (int32_t fd, const void* buf, int32_t nbytes)
 *   DESCRIPTION: Writes to the read_only file system, which is not a valid operation
 *   INPUTS: file descripter, the buffer, and the number of bytes to write
 *   OUTPUTS: return the number of bytes written
 *   SIDE EFFECTS: NONE
 */
int32_t write_dir (int32_t fd, const void* buf, int32_t nbytes){
    /* we should never write in a read-only file system */
    return -1;
}

