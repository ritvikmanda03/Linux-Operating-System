#include "x86_desc.h"
#include "file_sys.h"
#include "keyboard.h"

#define PCB_SIZE 8192
#define END_KERNEL 0x800000

/* halt system call */
int32_t halt (uint8_t status);
/* execute system call */
int32_t execute (const uint8_t* command);
/* read system call */
int32_t read (int32_t fd, void* buf, int32_t nbytes);
/* write system call */
int32_t write (int32_t fd, const void* buf, int32_t nbytes); 
/* open system call */
int32_t open (const uint8_t* filename);
/* close system call */
int32_t close (int32_t fd);
/* getargs system call */
int32_t getargs (uint8_t* buf, int32_t nbytes);
/* vidmap system call */
int32_t vidmap (uint8_t** screen_start);
/* set_handler system call */
int32_t set_handler (int32_t signum, void* handler_address);
/* sigreturn system call */
int32_t sigreturn (void);

/* finish execute function call */
void finish_execute(void* starting_address);
/* finish halt function call */
void finish_halt(uint32_t a, uint32_t b);
void finish_restore_exe(uint32_t a);
//extern void finish_execute(uint8_t[4]);

/* new read directory function */
int32_t read_dir_pcb (int32_t fd, void* buf, int32_t nbytes);
/* new read rtc function */
int32_t read_rtc_pcb (int32_t fd, void* buf, int32_t nbytes);
/* new close file function */
int32_t close_file_pcb (int32_t fd);
/* new close directory function */
int32_t close_dir_pcb (int32_t fd);

int get_terminal(int t);

int next_available_process();
void restoreExec(int pid);

/* pcb struct to type cast bottom of kernel memory for each process */
typedef struct pcb {
    /* define the file descriptor array */
    file_entry_t file_array[8];
    /* need to store ebp */
    uint32_t ebp;
    /* get arg call */
    uint8_t cur_arg[33];
    int process_id;
    int parent_id;
    uint32_t old_ebp;

} pcb_t;

