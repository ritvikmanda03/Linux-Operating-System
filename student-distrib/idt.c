#include "idt.h"


/* 
 *   idt_init
 *   DESCRIPTION: Initializes the IDT Table and fills in all struct value for each entry in the table
 *   INPUTS: none
 *   OUTPUTS: none
 *   Return: none
 */
void idt_init(){
        int i;
    for(i = 0; i < NUM_VEC; i++ ){
        //initialize all struct values 
        idt[i].seg_selector = KERNEL_CS;
        idt[i].reserved4    = 0;
        idt[i].reserved3    = 1; 
        idt[i].reserved2    = 1; 
        idt[i].reserved1    = 1;
        idt[i].size         = 1;
        idt[i].reserved0    = 0;
        idt[i].dpl          = 0;
        idt[i].present      = 0;

        if( i >= 0 && i <20){
            idt[i].present = 1; //set exceptions to present
            idt[i].reserved3 = 0;
        }
        if((i == 0x21) | (i == 0x28)){
            idt[i].present = 1; //set interrupts to present
        }
        if(i == 0x80){
            idt[i].dpl = 3; //priority for system call
            idt[i].present = 1; //set system call to present
            idt[i].reserved3 = 1;
        }
    }
    //SET_IDT_ENTRY FOR EXCEPTIONS
    SET_IDT_ENTRY(idt[0], Division_Error_asm);
    SET_IDT_ENTRY(idt[1], Debug_asm);
    SET_IDT_ENTRY(idt[2], NMI_asm);
    SET_IDT_ENTRY(idt[3], Breakpoint_asm);
    SET_IDT_ENTRY(idt[4], Overflow_asm);
    SET_IDT_ENTRY(idt[5], Bound_Range_Exceeded_asm);
    SET_IDT_ENTRY(idt[6], Invalid_Opcode_asm);
    SET_IDT_ENTRY(idt[7], Device_Not_Available_asm);
    SET_IDT_ENTRY(idt[8], Double_Fault_asm);
    SET_IDT_ENTRY(idt[9], Coprocessor_Segment_Overr_asm);
    SET_IDT_ENTRY(idt[10], Invalid_TSS_asm);
    SET_IDT_ENTRY(idt[11], Segment_Not_Present_asm);
    SET_IDT_ENTRY(idt[12], Stack_Segment_Fault_asm);
    SET_IDT_ENTRY(idt[13], General_Protection_Fault_asm);
    SET_IDT_ENTRY(idt[14], Page_Fault_asm);
    //15 is reserved
    SET_IDT_ENTRY(idt[16], x87_Floating_Point_Exception_asm);
    SET_IDT_ENTRY(idt[17], Alignment_Check_asm);
    SET_IDT_ENTRY(idt[18], Machine_Check_asm);
    SET_IDT_ENTRY(idt[19], SIMD_Floating_Point_exception_asm);

    //SET_IDT_ENTRY FOR INTERRUPTS
    SET_IDT_ENTRY(idt[0x21], keyboard_handler_asm);
    SET_IDT_ENTRY(idt[0x28], rtc_handler_asm);

    //SET_IDT_ENTRY FOR SYSTEM CALL
    SET_IDT_ENTRY(idt[0x80], idtSyscall_asm);
}
