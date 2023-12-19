#include "system_calls.h"
#include "page.h"
#include "keyboard.h"
#include "x86_desc.h"
#include "rtc.h"

#include "lib.h"

/* we hav 2 processes, we add one to determine which process we are at */
int process_index = -1; 

/* checkes to see how many processes are available to us */
int available_process[6] = {0, 0, 0, 0, 0, 0};

/* 
 *   halt()
 *   DESCRIPTION: Terminates a process, returning the status to the parent process.
 *   INPUTS: status - The exit status to be returned to the parent process.
 *   OUTPUTS: Returns 0 on success, or -1 on error.
 *   SIDE EFFECTS: May affect process table and system resources.
 */
int32_t halt (uint8_t status){
    cli();
    uint32_t retstat = status;
    /* determines the current process index */
    process_index = top_terminal_pid[terminal];

    if(process_index == 0 || process_index == 1 || process_index == 2){
        // return 0; // Ignore
        pcb_t* curr_pcb;
        curr_pcb = (pcb_t *) (END_KERNEL - ((process_index + 1) * (PCB_SIZE)));
        /* resets the process to available */
        top_terminal_pid[terminal] = -1;
        available_process[curr_pcb->process_id] = 0;
        sti();
        execute((const uint8_t*) "shell");
    }

    /* allocate and type cast the memory to a pcb struct */
    pcb_t* curr_pcb;
    curr_pcb = (pcb_t *) (END_KERNEL - ((process_index + 1) * (PCB_SIZE))); // PCB_SIZE is 8KB

    // clear fd array;
    int i;
    for(i=2; i<8; i++){
        curr_pcb->file_array[i].flags = 0;
    }

    /* allocate for the shell in memory */
    top_terminal_pid[terminal] = curr_pcb->parent_id;
    /* sets process to available*/
    available_process[curr_pcb->process_id] = 0;

    /* setting TSS */
    tss.ss0 = KERNEL_DS;
    tss.esp0 = END_KERNEL - ((curr_pcb->parent_id + 1) * (PCB_SIZE));

    /* reset squash flag if status reaches max IDT index = 255 */
    if(status == 255 && squashFlag){ 
        retstat = 256; /* 256 represents total number of IDT entries */
        squashFlag = 0;
    }

    process_index = curr_pcb->parent_id;

    program_paging(curr_pcb->parent_id);

    /* call finish halt function */
    sti();
    finish_halt(curr_pcb->ebp, retstat);
    return -1;

}

/* 
 *   execute()
 *   DESCRIPTION: Executes a new process, replacing the current process image with a new process image.
 *   INPUTS: command - The command string of the new process to execute.
 *   OUTPUTS: Returns -1 on error, or does not return on success.
 *   SIDE EFFECTS: The current process image is replaced, and new process starts execution.
 */
int32_t execute (const uint8_t* command) {
    // Declare variables
    unsigned i;
    unsigned j;

    // uint8_t * cur_cmd;
    // *cur_cmd = *(command);
    uint8_t cur_cmd[256];
    for(j=0; j<strlen((char*)(command)) + 1; j++){
        cur_cmd[j] = command[j];
    }
    //strncpy((char*)cur_cmd, (char*)command, strlen((char*)(command)) + 1);

    
    /* these magic numbers represent the ELF start for executable files */
    uint8_t magic_numbers[4] = {
        0x7f, 0x45, 0x4c, 0x46
    };
    /* max file name size + /0 */ /* max magic numbers */ /* starting address found in bytes 24-27*/
    uint8_t file_name[33], magic_numbers_found[4], starting_address[4];
    dentry_t file_info;
//old pcb stuff

    /* check if we have a valid command */
    if (command == NULL) {
        return -1;
    }

    // FIND THE FILE NAME
    i = 0;
    while(command[i] == ' '){ i+=1; }
    int temp = 0;
    while (command[i] != '\0' && command [i] != ' ' && i != 32) { // 31 max file name size
        file_name[temp] = command[i];
        i += 1;
        temp++;
    }
    file_name[temp] = '\0'; // Making the file name null terminating
    //i += 1;

//-------------old while loop

    // CHECK IF FILE EXISTS

    if (read_dentry_by_name (file_name, &file_info) == -1) { 
        return -1; 
    }

    // CHECK IF MAGIC NUMBERS EXIST

    if (read_data(file_info.inode_n, 0, magic_numbers_found, sizeof(magic_numbers)) != sizeof(magic_numbers)) {
        return -1;
    }
 
    if (strncmp((const char *)magic_numbers, (const char *)magic_numbers_found, sizeof(magic_numbers)) != 0) {
        return -1;
    }


    // The EIP you need to jump to is the entry point from bytes 24-27 of the executable that you have just loaded.

    if (read_data(file_info.inode_n, 24, starting_address, sizeof(starting_address)) != sizeof(starting_address)) {
        return -1;
    }

    // SET UP PAGING
    int old_process_index = top_terminal_pid[terminal];
    /* determines proces index */
    process_index = next_available_process();

    /* checks to see if process index exists or not */
    if (process_index != -1) {
        available_process[process_index] = 1;
        top_terminal_pid[terminal] = process_index;
    }
    else {
        return -1;
    }


    /* determine which process we need to allocate memory for */
    program_paging(process_index);

    // LOAD PROGRAM TO VIRTUAL ADDRESS

    read_data(file_info.inode_n, 0, (uint8_t *)0x08048000, 0x400000);

    pcb_t* curr_pcb;
    curr_pcb = (pcb_t *) (END_KERNEL - ((process_index + 1) * (PCB_SIZE))); // PCB_SIZE is 8KB

    curr_pcb->process_id = process_index;
    curr_pcb->parent_id = old_process_index;
    //start looking at argument with pcb
    uint8_t arg_length = 0;
    if(cur_cmd[i] != NULL){
        i += 1;
        curr_pcb->cur_arg[0] = NULL;
        while (cur_cmd[i] != '\0' && cur_cmd [i] != ' ' && i != 256) { // 31 max file name size
            curr_pcb->cur_arg[arg_length] = cur_cmd[i];
            i += 1;
            arg_length += 1;
        }
        curr_pcb->cur_arg[arg_length] = '\0';
    }
    /* set the arg to no arg essentially */
    else{
        curr_pcb->cur_arg[0] = NULL;
    }
    
    /* intialize the fd_array in PCB struct */
    curr_pcb->file_array[0].flags = 1;
    curr_pcb->file_array[1].flags = 1;
    curr_pcb->file_array[0].table_pointer.read = terminal_read;
    curr_pcb->file_array[1].table_pointer.write = terminal_write;

    /* initialize the rest of the FD to 0 */
    for (i = 2; i < 8; i++) {
        curr_pcb->file_array[i].flags = 0;
    }

    // asm volatile(
    //     "movl %%esp, %0;"
    //     "movl %%ebp, %1;"
    //     : "=r" (curr_pcb->esp), "=r" (curr_pcb->ebp)                           
    // );

    asm __volatile__(
        "movl %%ebp, %0;"
        : "=r"(curr_pcb->ebp)
    );

    /* setting TSS */
    tss.ss0 = KERNEL_DS;
    tss.esp0 = END_KERNEL - ((process_index + 1) * (PCB_SIZE));

    finish_execute(starting_address);
    /*  popl %edx
    orl $0x200, %edx
    pushl %edx */

    return 0;

}

/* 
 *   restoreExec(int pid)
 *   DESCRIPTION: restores old ebp upon calling execute function 
 *   INPUTS: int pid
 *   OUTPUTS: NONE
 *   SIDE EFFECTS: NONE
 */
void restoreExec(int pid) {
    /* get the current pcb */
    pcb_t* curr_pcb;
    curr_pcb = (pcb_t *) (END_KERNEL - ((pid + 1) * (PCB_SIZE)));
    /* set up TSS  */
    tss.ss0 = KERNEL_DS;
    tss.esp0 = END_KERNEL - ((curr_pcb->process_id + 1) * (PCB_SIZE));

    /* call the paging scheme */
    program_paging(pid);
    finish_restore_exe(curr_pcb->old_ebp);

}

/* 
 *   next_available_process()
 *   DESCRIPTION: Iterates through the top_terminal_pid and determines next available slot for a process
 *   to be run in
 *   INPUTS: NONE
 *   OUTPUTS: -1 on failure and process index
 *   SIDE EFFECTS: NONE
 */
int next_available_process() {
    int i = 0;
    /* check to see if terminal exists or not */
    if(top_terminal_pid[terminal] == -1) {
        /* keeps checking until one spot is open*/
        while (available_process[i] == 1) {
            i++;
            /* checks if reaches a spot stored for terminal */
            if (i == 3) {
                return -1;
            }
        }
    } else {
        /* start past spots reserved for terminals */
        i = 3;
        /* keeps checking for next available location */
        while (available_process[i] == 1) {
            i++;
            /* if alla re full return -1 */
            if (i == 6) {
                return -1;
            }
        }
        
    }
    /* return index of available process */
    return i;
    
}


/* 
 *   read_file (int32_t fd, void* buf, int32_t nbytes)
 *   DESCRIPTION: Calls the read data function to populate the buffer given the file index and the offset into the file.
 *                It updates the current position in the file after read is called.
 *   INPUTS: file descripter, the buffer, and the number of bytes to read
 *   OUTPUTS: return the number of bytes read
 *   SIDE EFFECTS: NONE
 */

int32_t read_file_pcb (int32_t fd, void* buf, int32_t nbytes) {
    /* type case pcb to memory location of current process */
    pcb_t* curr_pcb;
    curr_pcb = (pcb_t *) (END_KERNEL - ((process_index + 1) * (PCB_SIZE))); // PCB_SIZE is 8KB

    if (fd == 0){
        return terminal_read(fd, buf, nbytes);
    } else if (fd == 1){
        return -1;
    }   

    int32_t rv = read_data(curr_pcb->file_array[fd].inode_idx, curr_pcb->file_array[fd].position, buf, nbytes);
    /* update the current position in the file */
    curr_pcb->file_array[fd].position += rv;
    /* return # of bytes read */
    return rv;
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

int32_t read_dir_pcb (int32_t fd, void* buf, int32_t nbytes){
    /* type case pcb to memory location of current process */
    pcb_t* curr_pcb;
    curr_pcb = (pcb_t *) (END_KERNEL - ((process_index + 1) * (PCB_SIZE))); // PCB_SIZE is 8KB

    /* check to see if the current position/file index is valid */
    if (curr_pcb->file_array[fd].position >= boot_block->dir_entries_n){
        return 0;
    }
    /* create a temp dentry so we can read the filename by index */
    dentry_t temp_dentry;
    read_dentry_by_index(curr_pcb->file_array[fd].position, &temp_dentry);
    /* copy the file name into the buffer*/
    strncpy(buf, temp_dentry.file_name, 32);
    /* go to the next file in the directory entries */
    curr_pcb->file_array[fd].position++;
    /* return 1 for the number of files being read, once per read*/
    if(strlen(buf) > 32){
        return 32;
    }
    return strlen(buf); // curr_pcb->file_array[fd].position;
}

/* 
 *   close_file (int32_t fd)
 *   DESCRIPTION: detemines if a file can be closed or not given the file descriptor. Once it is able to,
 *                it sets the file back into not in use for the future.
 *   INPUTS: file descripter
 *   OUTPUTS: return value if it succeed or failed
 *   SIDE EFFECTS: NONE
 */

int32_t close_file_pcb (int32_t fd){
    /* type case pcb to memory location of current process */
    pcb_t* curr_pcb;
    curr_pcb = (pcb_t *) (END_KERNEL - ((process_index + 1) * (PCB_SIZE))); // PCB_SIZE is 8KB
    /* check to see if the file is not stdin or stdout */
    if(fd > 1 && fd < 8){
        if(curr_pcb->file_array[fd].flags == 0){
        return -1;
        }
        /* if it is not, then it is a regular file and we put it not into use */
        curr_pcb->file_array[fd].flags = 0;
        return 0;
    }
    /* return -1 on fail */
    return -1;
}

/* 
 *   close_dir (int32_t fd)
 *   DESCRIPTION: detemines if a directory can be closed or not given the file descriptor. Once it is able to,
 *                it sets the directory back into not in use for the future.
 *   INPUTS: file descripter
 *   OUTPUTS: return value if it succeed or failed
 *   SIDE EFFECTS: NONE
 */

int32_t close_dir_pcb (int32_t fd){
    /* type case pcb to memory location of current process */
    pcb_t* curr_pcb;
    curr_pcb = (pcb_t *) (END_KERNEL - ((process_index + 1) * (PCB_SIZE))); // PCB_SIZE is 8KB
    /* check to see if the directory is not stdin or stdout */
    if(fd > 1 && fd < 8){
        if(curr_pcb->file_array[fd].flags == 0){
        return -1;
        }
        /* if it is not, then it is a regular directory and we put it not into use */
        curr_pcb->file_array[fd].flags = 0;
        return 0;
    }
    /* return -1 on fail */
    return -1;
}

/* 
 *   read()
 *   DESCRIPTION: Reads data from a file descriptor into a buffer.
 *   INPUTS: fd - The file descriptor from which to read.
 *             buf - The buffer to store the read data.
 *             nbytes - The number of bytes to read.
 *   OUTPUTS: Returns the number of bytes read, or -1 on error.
 *   SIDE EFFECTS: The read operation may change the file's current position.
 */

int32_t read (int32_t fd, void* buf, int32_t nbytes){
    /* type case pcb to memory location of current process */
    pcb_t* curr_pcb;

    /* check for boundaries */
    if(fd < 0 || fd > 8 || fd == 1 || buf == NULL || nbytes < 0){return -1;}

    curr_pcb = (pcb_t *) (END_KERNEL - ((process_index + 1) * (PCB_SIZE))); // PCB_SIZE is 8KB

    /* check is flags are in use or not */
    if(curr_pcb->file_array[fd].flags == 0){return -1;}

    /* if successful, call the jumptabled read function */
    return curr_pcb->file_array[fd].table_pointer.read(fd, buf, nbytes);

}

/* 
 *   write()
 *   DESCRIPTION: Writes data to a file descriptor from a buffer.
 *   INPUTS: fd - The file descriptor to which to write.
 *             buf - The buffer containing the data to write.
 *             nbytes - The number of bytes to write.
 *   OUTPUTS: Returns the number of bytes written, or -1 on error.
 *   SIDE EFFECTS: The write operation may change the file's current position.
 */

int32_t write (int32_t fd, const void* buf, int32_t nbytes){
    /* type case pcb to memory location of current process */
    pcb_t* curr_pcb;

     /* check for boundaries */
    if(fd < 0 || fd > 8 || fd == 0 || buf == NULL || nbytes < 0){return -1;}

    curr_pcb = (pcb_t *) (END_KERNEL - ((process_index + 1) * (PCB_SIZE))); // PCB_SIZE is 8KB

    /* check is flags are in use or not */
    if(curr_pcb->file_array[fd].flags == 0){return -1;}

    /* if successful, call the jumptabled write function */
    return curr_pcb->file_array[fd].table_pointer.write(fd, buf, nbytes);
}

/* 
 *   open()
 *   DESCRIPTION: Opens a file, returning a file descriptor to the opened file.
 *   INPUTS: filename - The name of the file to open.
 *   OUTPUTS: Returns a non-negative file descriptor on success, or -1 on error.
 *   SIDE EFFECTS: Allocates a file descriptor and may affect the file descriptor table.
 */

int32_t open (const uint8_t* filename){
     /* type case pcb to memory location of current process */
    pcb_t* curr_pcb;
    dentry_t file_info;
    int32_t file_desc = 0;
    unsigned j;

    curr_pcb = (pcb_t *) (END_KERNEL - ((process_index + 1) * (PCB_SIZE))); // PCB_SIZE is 8KB

    /* keep the filename within 32 characters regardless of input length*/
    uint8_t fn[33];
    for(j=0; j<32; j++){
        fn[j] = filename[j];
    }
    fn[32] = '\0';
    
    while(curr_pcb->file_array[file_desc].flags == 1) { 
        file_desc++;  
        if (file_desc >= 8) {
            /* return -1 on fail if we got past the indexes of the array */
            return -1;
        } 
    }
    /* create a dentry and check to to see if we can find the file_name*/
    if (read_dentry_by_name (fn, &file_info) == -1) { 
        return -1; 
    }
    /* set up the file descriptor */
    curr_pcb->file_array[file_desc].inode_idx = file_info.inode_n;
    /* file is at the start */
    curr_pcb->file_array[file_desc].position = 0;
    /* file is now in use */
    curr_pcb->file_array[file_desc].flags = 1;


    /* set up the jumptables depending on the file type */
    if (file_info.file_type == 1) { // Opening Directory
        curr_pcb->file_array[file_desc].table_pointer.read = read_dir_pcb;
        curr_pcb->file_array[file_desc].table_pointer.write = write_dir;
        curr_pcb->file_array[file_desc].table_pointer.close = close_dir_pcb;
        curr_pcb->file_array[file_desc].table_pointer.open = open_dir; 

    } else if (file_info.file_type == 2) {    // Opening File

        curr_pcb->file_array[file_desc].table_pointer.read = read_file_pcb;
        curr_pcb->file_array[file_desc].table_pointer.write = write_file;
        curr_pcb->file_array[file_desc].table_pointer.close = close_file_pcb;
        curr_pcb->file_array[file_desc].table_pointer.open = open_file;

    } else if (file_info.file_type == 0) {    // Opening Rtc

        curr_pcb->file_array[file_desc].table_pointer.read = rtc_read;
        curr_pcb->file_array[file_desc].table_pointer.write = rtc_write;
        curr_pcb->file_array[file_desc].table_pointer.close = rtc_close;
        curr_pcb->file_array[file_desc].table_pointer.open = rtc_open; 
    }

    /* return FD*/

    return file_desc;
}

/* 
 *   close()
 *   DESCRIPTION: Closes an opened file descriptor.
 *   INPUTS: fd - The file descriptor to close.
 *   OUTPUTS: Returns 0 on success, or -1 on error.
 *   SIDE EFFECTS: Releases the file descriptor and its associated resources.
 */

int32_t close (int32_t fd){
    /* type cast pcb to memory location of current process */

    /* check for valid fd index */
    if(fd < 2 || fd > 8){return -1;}

    pcb_t* curr_pcb;
    curr_pcb = (pcb_t *) (END_KERNEL - ((process_index + 1) * (PCB_SIZE))); // PCB_SIZE is 8KB

    /* check is flags are in use or not */
    if(curr_pcb->file_array[fd].flags == 0){return -1;}

    /* call the specified close function */
    if (curr_pcb->file_array[fd].table_pointer.close(fd) == -1) {
        return -1;
    }
    curr_pcb->file_array[fd].flags = 0;
    return 0;
}

//push
/* 
 *   getargs (uint8_t* buf, int32_t nbytes)
 *   DESCRIPTION: Reads the program's command line arguments into a user-level buffer
 *   INPUTS: uint8_t* buf, int32_t nbytes
 *   OUTPUTS: return -1 if args do not fit in the buffer
 *   SIDE EFFECTS: NONE
 */

int32_t getargs (uint8_t* buf, int32_t nbytes){
    /* get current pcb by type casting */
    pcb_t* curr_pcb;
    curr_pcb = (pcb_t *) (END_KERNEL - ((process_index + 1) * (PCB_SIZE)));

    /* check if there is even an argument or not */
    if (curr_pcb->cur_arg[0] == NULL){ return -1; }
    /* check if size of arg is valid */
    if(strlen((char*)(curr_pcb->cur_arg)) > nbytes){ return -1;}
    /*if there is, copy it into the buffer */
    strncpy((char*)buf, (char*) curr_pcb->cur_arg, strlen((char*)(curr_pcb->cur_arg)) + 1);
    /* reset the argument after it is used */
    curr_pcb->cur_arg[0] = NULL;
    return 0;
}
/* 
 *   vidmap (uint8_t** screen_start)
 *   DESCRIPTION: Calls maps the video mem in user space at a pre set virtual address
 *   INPUTS: uint8_t** screen_start
 *   OUTPUTS: Address, else -1 if the location is not valid 
 *   SIDE EFFECTS: assigns the screen start pointer to the vidmap address
 */
int32_t vidmap (uint8_t** screen_start){
    /* check if pointer is NULL or not */
    if(screen_start == NULL){return -1;}
    /* check is the screen start pointer is within the correct bounds */
    if(((uint32_t)screen_start > USER_SPACE + KERNEL_ADDRESS) || ((uint32_t)screen_start < USER_SPACE)){ return -1;}

    /* helper function to set up paging */
    //memcpy(get_terminal(terminal), (void *)VIDEO_MEM_ADDRESS, 4096);
    vidmap_paging((int8_t **)screen_start, terminal);
    //memcpy((void *)VIDEO_MEM_ADDRESS, get_terminal(terminal), 4096);

    /* return 0 on success, should always happen */
    return 0;
}
/* 
 *   set_handler (int32_t signum, void* handler_address)
 *   DESCRIPTION: Sets the handler in response to various events or signals sent to process
 *   INPUTS: int32_t signum, void* handler_address
 *   OUTPUTS: -1 on failure
 *   SIDE EFFECTS: NONE
 */
int32_t set_handler (int32_t signum, void* handler_address){
    return 0;
}
/* 
 *   sigreturn (void)
 *   DESCRIPTION: Returns the signal 
 *   INPUTS: void
 *   OUTPUTS: -1 on failure
 *   SIDE EFFECTS: NONE
 */
int32_t sigreturn (void){
    return 0;
}
