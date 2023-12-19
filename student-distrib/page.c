#include "page.h"

//align page directory and table by their number of entries (4*1024 = 4096)
pd_desc_t pd[1024] __attribute__((aligned(4 * 1024)));
pt_desc_t pt[1024] __attribute__((aligned(4 * 1024)));
pt_desc_t vm[1024] __attribute__((aligned(4 * 1024)));



//inline assembly to set control register 3 to the page 
//directory address, set the page extension bit of control
//register 4, and turn on paging as supervisor in control
//register 0.
/* 
 *   page_setup()
 *   DESCRIPTION: Sets the corresponding needed bits to CR3, CR0, and CR4 CPU registers
 *   INPUTS: none
 *   OUTPUTS: none
 *   SIDE EFFECTS: CR3, CR0, and CR4 hardware registers are changed 
 */
void page_setup(){                      
    asm volatile(   
        "mov %0, %%eax;"                              
        "mov %%eax, %%cr3;" 

        "mov %%cr4, %%eax;"                
        "or $0x00000010, %%eax;"         
        "mov %%eax, %%cr4;"                
                                        
        "mov %%cr0, %%eax;"                
        "or $0x80000001, %%eax;"
        "mov %%eax, %%cr0;"         
                                        
        : : "r"(pd) : "%eax"             
    );
}

/* 
 *   page_setup_paging()
 *   DESCRIPTION: CR3 is reloaded so that TLBs can be flushed
 *   INPUTS: none
 *   OUTPUTS: none
 *   SIDE EFFECTS: TLBs can be flushed 
 */
void page_setup_paging() {
    asm volatile(   
        "mov %0, %%eax;"                              
        "mov %%eax, %%cr3;"       
                                        
        : : "r"(pd) : "%eax"             
    );
}

//macro to find index into page directory
#define PDindex(p)  p>>22
//macro to find index into page table
#define PTindex(p)  p>>12 & 0x3FF

// array to store addresses for each terminal 
int VIDEO_PAGE_ADDRESSES[4] = { VIDEO_MEM_ADDRESS, VIDEO_PAGE_ONE, VIDEO_PAGE_TWO, VIDEO_PAGE_THREE };


void start_paging() {
   // printf("Kernel paging");
   
    //assign kernel address to upper 10 bits, set PS (page size), set P (present bit), set RW (read/write)
    pd[PDindex(KERNEL_ADDRESS)].val = (KERNEL_ADDRESS & 0xE00000);  //only set first three bits as kernel address
    pd[PDindex(KERNEL_ADDRESS)].p = 1;
    pd[PDindex(KERNEL_ADDRESS)].rw = 1;
    pd[PDindex(KERNEL_ADDRESS)].ps = 1;
    pd[PDindex(KERNEL_ADDRESS)].g = 1;

    //assign page table address, set US (user/supervisor),  set P (present bit), set RW (read/write)
    pd[PDindex(VIDEO_MEM_ADDRESS)].val = (unsigned int)pt;
    pd[PDindex(VIDEO_MEM_ADDRESS)].p = 1;
    pd[PDindex(VIDEO_MEM_ADDRESS)].rw = 1;
    pd[PDindex(VIDEO_MEM_ADDRESS)].us = 1;

    //assign video memory address, set US (user/supervisor),  set P (present bit), set RW (read/write)
    pt[PTindex(VIDEO_MEM_ADDRESS)].val = VIDEO_MEM_ADDRESS;
    pt[PTindex(VIDEO_MEM_ADDRESS)].p = 1;
    pt[PTindex(VIDEO_MEM_ADDRESS)].rw = 1;
    pt[PTindex(VIDEO_MEM_ADDRESS)].us = 0; // 1 because user should have access to video memory, else 0

    // assign video mem page for terminal 1
    pt[PTindex(VIDEO_PAGE_ONE)].val = VIDEO_PAGE_ONE;
    pt[PTindex(VIDEO_PAGE_ONE)].p = 1;
    pt[PTindex(VIDEO_PAGE_ONE)].rw = 1;
    pt[PTindex(VIDEO_PAGE_ONE)].us = 0; // 1 because user should have access to video memory, else 0

    // assign video mem page for terminal 1
    pt[PTindex(VIDEO_PAGE_TWO)].val = VIDEO_PAGE_TWO;
    pt[PTindex(VIDEO_PAGE_TWO)].p = 1;
    pt[PTindex(VIDEO_PAGE_TWO)].rw = 1;
    pt[PTindex(VIDEO_PAGE_TWO)].us = 0; // 1 because user should have access to video memory, else 0
    
    // assign video mem page for terminal 1
    pt[PTindex(VIDEO_PAGE_THREE)].val = VIDEO_PAGE_THREE;
    pt[PTindex(VIDEO_PAGE_THREE)].p = 1;
    pt[PTindex(VIDEO_PAGE_THREE)].rw = 1;
    pt[PTindex(VIDEO_PAGE_THREE)].us = 0; // 1 because user should have access to video memory, else 0

    page_setup(); 
}

/* 
 *   program_paging(uint8_t pid)
 *   DESCRIPTION: Remaps the address for program virtual address in virtual memory to a different 
 *   location in phsycial memory based on the process we are at, this is in multiples of 4MB
 *   after 8MB kernel page 
 *   INPUTS: uint8_t pid
 *   OUTPUTS: none
 *   SIDE EFFECTS: program virtual address gets mapped to a diffrent address in physical memory
 */
void program_paging(uint8_t pid) {

    // maps to the correspodning physical memory address based on whichp process we are at
    pd[PDindex(PROG_VIR_ADDRESS)].val = ((PROGRAM_ADDRESS + (0x400000 * pid)) & 0xE00000);  //only set first three bits as shell address
    pd[PDindex(PROG_VIR_ADDRESS)].p = 1;
    pd[PDindex(PROG_VIR_ADDRESS)].rw = 1;
    pd[PDindex(PROG_VIR_ADDRESS)].ps = 1;
    pd[PDindex(PROG_VIR_ADDRESS)].us = 1;

    /* clears TLBS, sets up normal paging scheme */
    page_setup_paging();
}


/* 
 *   vidmap_paging()
 *   DESCRIPTION: Initializes the table and directory with the correct values for their entries for vidmap
 *   INPUTS: screenstart
 *   OUTPUTS: none
 *   SIDE EFFECTS: assigns the screen start pointer to the vidmap address
 */
void vidmap_paging(int8_t** screen_start, int terminal){
    //assign page table address, set US (user/supervisor),  set P (present bit)
    pd[PDindex(VIDMAP_ADDRESS)].val = (unsigned int)vm;
    pd[PDindex(VIDMAP_ADDRESS)].p = 1;
    pd[PDindex(VIDMAP_ADDRESS)].us = 1;
    pd[PDindex(VIDMAP_ADDRESS)].rw = 1;
    //assign video memory address, set US (user/supervisor),  set P (present bit)
    vm[0].val = VIDEO_MEM_ADDRESS;
    vm[0].p = 1;
    vm[0].us = 1;
    vm[0].rw = 1;

    /*assigns the screen start pointer to the vidmap address */
    *screen_start = (int8_t *)VIDMAP_ADDRESS;

    /* clears TLBS, sets up normal paging scheme */
    page_setup_paging();
}


