/* x86_desc.h - Defines for various x86 descriptors, descriptor tables,
 * and selectors
 * vim:ts=4 noexpandtab
 */

#ifndef _X86_DESC_H
#define _X86_DESC_H

#include "types.h"

/* Segment selector values */
#define KERNEL_CS   0x0010
#define KERNEL_DS   0x0018
#define USER_CS     0x0023
#define USER_DS     0x002B
#define KERNEL_TSS  0x0030
#define KERNEL_LDT  0x0038

/* Size of the task state segment (TSS) */
#define TSS_SIZE    104

/* Number of vectors in the interrupt descriptor table (IDT) */
#define NUM_VEC     256

#ifndef ASM

volatile int squashFlag;

//Exceptions
void Division_Error();
void Debug();
void NMI();
void Breakpoint();
void Overflow();
void Bound_Range_Exceeded();
void Invalid_Opcode();
void Device_Not_Available();
void Double_Fault();
void Coprocessor_Segment_Overr();
void Invalid_TSS();
void Segment_Not_Present();
void Stack_Segment_Fault();
void General_Protection_Fault();
void Page_Fault();
void x87_Floating_Point_Exception();
void Alignment_Check();
void Machine_Check();
void SIMD_Floating_Point_exception();

//interrupts
void keyboard_handler();
void rtc_handler();
//system call
void idtSyscall();

//-------ASM--------
//Exceptions
void Division_Error_asm();
void Debug_asm();
void NMI_asm();
void Breakpoint_asm();
void Overflow_asm();
void Bound_Range_Exceeded_asm();
void Invalid_Opcode_asm();
void Device_Not_Available_asm();
void Double_Fault_asm();
void Coprocessor_Segment_Overr_asm();
void Invalid_TSS_asm();
void Segment_Not_Present_asm();
void Stack_Segment_Fault_asm();
void General_Protection_Fault_asm();
void Page_Fault_asm();
void x87_Floating_Point_Exception_asm();
void Alignment_Check_asm();
void Machine_Check_asm();
void SIMD_Floating_Point_exception_asm();


//interrupts
void keyboard_handler_asm();
void rtc_handler_asm();
//system call
void idtSyscall_asm();


/* This structure is used to load descriptor base registers
 * like the GDTR and IDTR */
typedef struct x86_desc {
    uint16_t padding;
    uint16_t size;
    uint32_t addr;
} x86_desc_t;

/* This is a segment descriptor.  It goes in the GDT. */
typedef struct seg_desc {
    union {
        uint32_t val[2];
        struct {
            uint16_t seg_lim_15_00;
            uint16_t base_15_00;
            uint8_t  base_23_16;
            uint32_t type          : 4;
            uint32_t sys           : 1;
            uint32_t dpl           : 2;
            uint32_t present       : 1;
            uint32_t seg_lim_19_16 : 4;
            uint32_t avail         : 1;
            uint32_t reserved      : 1;
            uint32_t opsize        : 1;
            uint32_t granularity   : 1;
            uint8_t  base_31_24;
        } __attribute__ ((packed));
    };
} seg_desc_t;

/* TSS structure */
typedef struct __attribute__((packed)) tss_t {
    uint16_t prev_task_link;
    uint16_t prev_task_link_pad;

    uint32_t esp0;
    uint16_t ss0;
    uint16_t ss0_pad;

    uint32_t esp1;
    uint16_t ss1;
    uint16_t ss1_pad;

    uint32_t esp2;
    uint16_t ss2;
    uint16_t ss2_pad;

    uint32_t cr3;

    uint32_t eip;
    uint32_t eflags;

    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;

    uint16_t es;
    uint16_t es_pad;

    uint16_t cs;
    uint16_t cs_pad;

    uint16_t ss;
    uint16_t ss_pad;

    uint16_t ds;
    uint16_t ds_pad;

    uint16_t fs;
    uint16_t fs_pad;

    uint16_t gs;
    uint16_t gs_pad;

    uint16_t ldt_segment_selector;
    uint16_t ldt_pad;

    uint16_t debug_trap : 1;
    uint16_t io_pad     : 15;
    uint16_t io_base_addr;
} tss_t;

/* Some external descriptors declared in .S files */
extern x86_desc_t gdt_desc;

extern uint16_t ldt_desc;
extern uint32_t ldt_size;
extern seg_desc_t ldt_desc_ptr;
extern seg_desc_t gdt_ptr;
extern uint32_t ldt;

extern uint32_t tss_size;
extern seg_desc_t tss_desc_ptr;
extern tss_t tss;

/* Sets runtime-settable parameters in the GDT entry for the LDT */
#define SET_LDT_PARAMS(str, addr, lim)                          \
do {                                                            \
    str.base_31_24 = ((uint32_t)(addr) & 0xFF000000) >> 24;     \
    str.base_23_16 = ((uint32_t)(addr) & 0x00FF0000) >> 16;     \
    str.base_15_00 = (uint32_t)(addr) & 0x0000FFFF;             \
    str.seg_lim_19_16 = ((lim) & 0x000F0000) >> 16;             \
    str.seg_lim_15_00 = (lim) & 0x0000FFFF;                     \
} while (0)

/* Sets runtime parameters for the TSS */
#define SET_TSS_PARAMS(str, addr, lim)                          \
do {                                                            \
    str.base_31_24 = ((uint32_t)(addr) & 0xFF000000) >> 24;     \
    str.base_23_16 = ((uint32_t)(addr) & 0x00FF0000) >> 16;     \
    str.base_15_00 = (uint32_t)(addr) & 0x0000FFFF;             \
    str.seg_lim_19_16 = ((lim) & 0x000F0000) >> 16;             \
    str.seg_lim_15_00 = (lim) & 0x0000FFFF;                     \
} while (0)

/* An interrupt descriptor entry (goes into the IDT) */
typedef union idt_desc_t {
    uint32_t val[2];
    struct {
        uint16_t offset_15_00;
        uint16_t seg_selector;
        uint8_t  reserved4;
        uint32_t reserved3 : 1;
        uint32_t reserved2 : 1;
        uint32_t reserved1 : 1;
        uint32_t size      : 1;
        uint32_t reserved0 : 1;
        uint32_t dpl       : 2;
        uint32_t present   : 1;
        uint16_t offset_31_16;
    } __attribute__ ((packed));
} idt_desc_t;

/* The IDT itself (declared in x86_desc.S */
extern idt_desc_t idt[NUM_VEC];
/* The descriptor used to load the IDTR */
extern x86_desc_t idt_desc_ptr;

//*****************PAGE_DIRECTORY*********************
/* A page directory entry */
typedef union pd_desc_t {    
    uint32_t val;
    struct {
        uint32_t p          : 1;        //present bit
        uint32_t rw         : 1;        //read/write bit
        uint32_t us         : 1;        //user/supervisor bit
        uint32_t pwt        : 1;        //write-through bit
        uint32_t pcd        : 1;        //cache disable bit
        uint32_t a          : 1;        //accessed bit
        uint32_t avl        : 1;        //available
        uint32_t ps         : 1;        //page size bit
        uint32_t g          : 1;        //global overlap
        uint32_t avl_1      : 3;        //available overlap
        uint32_t addr_31_12 : 20;       //address of page table or 4MB page
    } __attribute__ ((packed));
} pd_desc_t;

/* The PD itself (declared in x86_desc.S) */

//****************PAGE_DIRECTORY_END********************

//********************PAGE_TABLE************************
/* A page table entry */
typedef union pt_desc_t {
    uint32_t val;
    struct {
        uint32_t p          : 1;        //present bit
        uint32_t rw         : 1;        //read/write bit
        uint32_t us         : 1;        //user/supervisor bit
        uint32_t pwt        : 1;        //write-through bit
        uint32_t pcd        : 1;        //cache disable bit
        uint32_t a          : 1;        //accessed bit
        uint32_t d          : 1;        //dirty bit
        uint32_t pat        : 1;        //page attr table bit
        uint32_t g          : 1;        //global bit
        uint32_t avl        : 3;        //available
        uint32_t addr_31_12 : 20;       //address of 4KB page
    } __attribute__ ((packed));
} pt_desc_t;

/* The PT itself (declared in x86_desc.S) */


/* Sets runtime parameters for an IDT entry */
#define SET_IDT_ENTRY(str, handler)                              \
do {                                                             \
    str.offset_31_16 = ((uint32_t)(handler) & 0xFFFF0000) >> 16; \
    str.offset_15_00 = ((uint32_t)(handler) & 0xFFFF);           \
} while (0)

/* Load task register.  This macro takes a 16-bit index into the GDT,
 * which points to the TSS entry.  x86 then reads the GDT's TSS
 * descriptor and loads the base address specified in that descriptor
 * into the task register */
#define ltr(desc)                       \
do {                                    \
    asm volatile ("ltr %w0"             \
            :                           \
            : "r" (desc)                \
            : "memory", "cc"            \
    );                                  \
} while (0)

/* Load the interrupt descriptor table (IDT).  This macro takes a 32-bit
 * address which points to a 6-byte structure.  The 6-byte structure
 * (defined as "struct x86_desc" above) contains a 2-byte size field
 * specifying the size of the IDT, and a 4-byte address field specifying
 * the base address of the IDT. */
#define lidt(desc)                      \
do {                                    \
    asm volatile ("lidt (%0)"           \
            :                           \
            : "g" (desc)                \
            : "memory"                  \
    );                                  \
} while (0)

/* Load the local descriptor table (LDT) register.  This macro takes a
 * 16-bit index into the GDT, which points to the LDT entry.  x86 then
 * reads the GDT's LDT descriptor and loads the base address specified
 * in that descriptor into the LDT register */
#define lldt(desc)                      \
do {                                    \
    asm volatile ("lldt %%ax"           \
            :                           \
            : "a" (desc)                \
            : "memory"                  \
    );                                  \
} while (0)






#endif /* ASM */

#endif /* _x86_DESC_H */
