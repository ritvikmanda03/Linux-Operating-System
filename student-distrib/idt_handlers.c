// #include "x86_desc.h"
// #include "types.h"
//maybe dont need all of the ones below but solves the printf problems
#include "multiboot.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "debug.h"
#include "tests.h"
#include "keyboard.h"
#include "rtc.h"
#include "system_calls.h"

#define MAX_IDT_ENTRY 255

//Exception hhandlers below. Clear the screen, print a value, and while loop to keep it stuck in the exception

/* 
 *   Division_Error
 *   DESCRIPTION: Clear the screen, prints the current exception, and while loop to keep it stuck in the exception
 *   INPUTS: none
 *   OUTPUTS: none
 *   Return: none
 */
void Division_Error(){
     
    printf("Exception occurred: Division_Error");
    halt(MAX_IDT_ENTRY);
}

/* 
 *   Debug
 *   DESCRIPTION: Clear the screen, prints the current exception, and while loop to keep it stuck in the exception
 *   INPUTS: none
 *   OUTPUTS: none
 *   Return: none
 */
void Debug(){
    printf("Exception occurred: Debug");
    halt(MAX_IDT_ENTRY);
    
}

/* 
 *   NMI
 *   DESCRIPTION: Clear the screen, prints the current exception, and while loop to keep it stuck in the exception
 *   INPUTS: none
 *   OUTPUTS: none
 *   Return: none
 */
void NMI(){
     
    printf("Exception occurred: NMI");
    halt(MAX_IDT_ENTRY);
}

/* 
 *   Breakpoint
 *   DESCRIPTION: Clear the screen, prints the current exception, and while loop to keep it stuck in the exception
 *   INPUTS: none
 *   OUTPUTS: none
 *   Return: none
 */
void Breakpoint(){
     
    printf("Exception occurred: Breakpoint");
    halt(MAX_IDT_ENTRY);
}

/* 
 *   Overflow
 *   DESCRIPTION: Clear the screen, prints the current exception, and while loop to keep it stuck in the exception
 *   INPUTS: none
 *   OUTPUTS: none
 *   Return: none
 */
void Overflow(){
     
    printf("Exception occurred: Overflow");
    halt(MAX_IDT_ENTRY);
}

/* 
 *   Bound_Range_Exceeded
 *   DESCRIPTION: Clear the screen, prints the current exception, and while loop to keep it stuck in the exception
 *   INPUTS: none
 *   OUTPUTS: none
 *   Return: none
 */
void Bound_Range_Exceeded(){
     
    printf("Exception occurred: Bound_Range_Exceeded");
    halt(MAX_IDT_ENTRY);
}

/* 
 *   Invalid_opcode
 *   DESCRIPTION: Clear the screen, prints the current exception, and while loop to keep it stuck in the exception
 *   INPUTS: none
 *   OUTPUTS: none
 *   Return: none
 */
void Invalid_Opcode(){
     
    printf("Exception occurred: Invalid_Opcode");
    halt(MAX_IDT_ENTRY);
}

/* 
 *   Device_not_available
 *   DESCRIPTION: Clear the screen, prints the current exception, and while loop to keep it stuck in the exception
 *   INPUTS: none
 *   OUTPUTS: none
 *   Return: none
 */
void Device_Not_Available(){
     
    printf("Exception occurred: Device_Not_Available");
    halt(MAX_IDT_ENTRY);
}

/* 
 *   Double_fault
 *   DESCRIPTION: Clear the screen, prints the current exception, and while loop to keep it stuck in the exception
 *   INPUTS: none
 *   OUTPUTS: none
 *   Return: none
 */
void Double_Fault(){
     
    printf("Exception occurred: Double_Fault");
    halt(MAX_IDT_ENTRY);
}

/* 
 *   Coprocessor_segment_overr
 *   DESCRIPTION: Clear the screen, prints the current exception, and while loop to keep it stuck in the exception
 *   INPUTS: none
 *   OUTPUTS: none
 *   Return: none
 */
void Coprocessor_Segment_Overr(){
     
    printf("Exception occurred: Coprocessor_Segment_Overr");
    halt(MAX_IDT_ENTRY);
}

/* 
 *   Invalid_tss
 *   DESCRIPTION: Clear the screen, prints the current exception, and while loop to keep it stuck in the exception
 *   INPUTS: none
 *   OUTPUTS: none
 *   Return: none
 */
void Invalid_TSS(){
     
    printf("Exception occurred: Invalid_TSS");
    halt(MAX_IDT_ENTRY);
}

/* 
 *   Segment_not_present
 *   DESCRIPTION: Clear the screen, prints the current exception, and while loop to keep it stuck in the exception
 *   INPUTS: none
 *   OUTPUTS: none
 *   Return: none
 */
void Segment_Not_Present(){
     
    printf("Exception occurred: Segment_Not_Present");
    halt(MAX_IDT_ENTRY);
}

/* 
 *   Stack_Segment_Fault
 *   DESCRIPTION: Clear the screen, prints the current exception, and while loop to keep it stuck in the exception
 *   INPUTS: none
 *   OUTPUTS: none
 *   Return: none
 */
void Stack_Segment_Fault(){
     
    printf("Exception occurred: Stack_Segment_Fault");
    halt(MAX_IDT_ENTRY);
}

/* 
 *   General_Protection_Fault
 *   DESCRIPTION: Clear the screen, prints the current exception, and while loop to keep it stuck in the exception
 *   INPUTS: none
 *   OUTPUTS: none
 *   Return: none
 */
void General_Protection_Fault(){
    printf("Exception occurred: General_Protection_Fault \n");
    squashFlag = 1;
    halt(MAX_IDT_ENTRY);
}

/* 
 *   Page_Fault
 *   DESCRIPTION: Clear the screen, prints the current exception, and while loop to keep it stuck in the exception
 *   INPUTS: none
 *   OUTPUTS: none
 *   Return: none
 */
void Page_Fault(){
    squashFlag = 1;
    printf("Exception occurred: Page_Fault \n");
    halt(MAX_IDT_ENTRY);
}

/* 
 *   x87_Floating_Point_Exception
 *   DESCRIPTION: Clear the screen, prints the current exception, and while loop to keep it stuck in the exception
 *   INPUTS: none
 *   OUTPUTS: none
 *   Return: none
 */
void x87_Floating_Point_Exception(){
    printf("Exception occurred: x87_Floating_Point_Exception");
    halt(MAX_IDT_ENTRY);
}

/* 
 *   Alignment_Check
 *   DESCRIPTION: Clear the screen, prints the current exception, and while loop to keep it stuck in the exception
 *   INPUTS: none
 *   OUTPUTS: none
 *   Return: none
 */
void Alignment_Check(){
     
    printf("Exception occurred: Alignment_Check");
    halt(MAX_IDT_ENTRY);
}

/* 
 *   Machine_Check
 *   DESCRIPTION: Clear the screen, prints the current exception, and while loop to keep it stuck in the exception
 *   INPUTS: none
 *   OUTPUTS: none
 *   Return: none
 */
void Machine_Check(){
     
    printf("Exception occurred: Machine_Check");
    halt(MAX_IDT_ENTRY);
}

/* 
 *   SIMD_Floating_Point_exception
 *   DESCRIPTION: Clear the screen, prints the current exception, and while loop to keep it stuck in the exception
 *   INPUTS: none
 *   OUTPUTS: none
 *   Return: none
 */
void SIMD_Floating_Point_exception(){
     
    printf("Exception occurred: SIMD_Floating_Point_exception");
    halt(MAX_IDT_ENTRY);
}

// // Treat system call as an exception for this cp
// /* 
//  *   idtSyscall
//  *   DESCRIPTION: Clear the screen, prints the current exception, and while loop to keep it stuck in the exception
//  *   INPUTS: none
//  *   OUTPUTS: none
//  *   Return: none
//  */
// void idtSyscall(){
//      
//     printf("SYSTEM CALL INVOKED");
//     halt(MAX_IDT_ENTRY);
// }
