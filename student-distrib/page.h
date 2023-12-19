#include "x86_desc.h"

#define DIRECTORY_SIZE 1024             // number of entries in the directory and table
#define TABLE_SIZE DIRECTORY_SIZE

#define   VIDEO_MEM_ADDRESS   0xB8000   //video memory location in hex
#define   VIDEO_PAGE_ONE      0xBA000
#define   VIDEO_PAGE_TWO      0xBC000   
#define   VIDEO_PAGE_THREE    0xBE000   
#define   KERNEL_ADDRESS      0x400000  //kernel memory location (4MB) in hex
#define   FOUR_KB_SIZE        4096      
#define   SHELL_ADDRESS       0x800000
#define   EXECUTE_ADDRESS     0xC00000

#define   PROGRAM_ADDRESS     0x800000

#define   PROG_VIR_ADDRESS    0x08000000
#define   VIDMAP_ADDRESS      0x8400000 
#define   USER_SPACE          0x8000000

extern pd_desc_t pd[1024];              //initialize array of page directory entry structs

extern pt_desc_t pt[1024];              //initialize array of page table entry structs

extern pt_desc_t vm[1024];              //initialize array of page table entry structs for video memory

int VIDEO_PAGE_ADDRESSES[4];


/* set up the paging by setting bits for linux registers */
void page_setup();       
void page_setup_paging();               
void start_paging();
void program_paging(uint8_t pid);
void vidmap_paging(int8_t** screen_start, int terminal);

