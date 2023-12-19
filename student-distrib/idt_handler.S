#define ASM     1
//set all asm functions to global so we can link them to the c functions
.globl Division_Error_asm, Debug_asm, NMI_asm, Breakpoint_asm, Overflow_asm, Bound_Range_Exceeded_asm, Invalid_Opcode_asm, Device_Not_Available_asm, Double_Fault_asm, Coprocessor_Segment_Overr_asm, Invalid_TSS_asm, Segment_Not_Present_asm, Stack_Segment_Fault_asm, General_Protection_Fault_asm, Page_Fault_asm, x87_Floating_Point_Exception_asm, Alignment_Check_asm, Machine_Check_asm, SIMD_Floating_Point_exception_asm, keyboard_handler_asm, rtc_handler_asm, idtSyscall_asm

//If any assembly function is called then call the corresponding C function. Must push all registers+flags then iret as that is an interrupt return
Division_Error_asm:
    pushal
    pushfl
    call Division_Error
    popfl
    popal
    iret 
    
Debug_asm:
    pushal
    pushfl
    call Debug
    popfl
    popal
    iret 
    
NMI_asm:
    pushal
    pushfl
    call NMI
    popfl
    popal
    iret 
    
Breakpoint_asm:
    pushal
    pushfl
    call Breakpoint
    popfl
    popal
    iret 
    
Overflow_asm:
    pushal
    pushfl
    call Overflow
    popfl
    popal
    iret 
    
Bound_Range_Exceeded_asm:
    pushal
    pushfl
    call Bound_Range_Exceeded
    popfl
    popal
    iret 
    
Invalid_Opcode_asm:
    pushal
    pushfl
    call Invalid_Opcode
    popfl
    popal
    iret 
    
Device_Not_Available_asm:
    pushal
    pushfl
    call Device_Not_Available
    popfl
    popal
    iret 
    
Double_Fault_asm:
    pushal
    pushfl
    call Double_Fault
    popfl
    popal
    iret 
    
Coprocessor_Segment_Overr_asm:
    pushal
    pushfl
    call Coprocessor_Segment_Overr
    popfl
    popal
    iret 
    
Invalid_TSS_asm:
    pushal
    pushfl
    call Invalid_TSS
    popfl
    popal
    iret 
    
Segment_Not_Present_asm:
    pushal
    pushfl
    call Segment_Not_Present
    popfl
    popal
    iret 
    
Stack_Segment_Fault_asm:
    pushal
    pushfl
    call Stack_Segment_Fault
    popfl
    popal
    iret 
    
General_Protection_Fault_asm:
    pushal
    pushfl
    call General_Protection_Fault
    popfl
    popal
    iret 
    
Page_Fault_asm:
    pushal
    pushfl
    call Page_Fault
    popfl
    popal
    iret 
    
x87_Floating_Point_Exception_asm:
    pushal
    pushfl
    call x87_Floating_Point_Exception
    popfl
    popal
    iret 
    
Alignment_Check_asm:
    pushal
    pushfl
    call Alignment_Check
    popfl
    popal
    iret 
    
Machine_Check_asm:
    pushal
    pushfl
    call Machine_Check
    popfl
    popal
    iret 
    
SIMD_Floating_Point_exception_asm:
    pushal
    pushfl
    call SIMD_Floating_Point_exception
    popfl
    popal
    iret 
        
keyboard_handler_asm:
    pushal
    pushfl
    call keyboard_handler
    popfl
    popal
    iret 
    
rtc_handler_asm:
    pushal
    pushfl
    call rtc_handler
    popfl
    popal
    iret 
// idt_jumptable for all of the system call functions we defined
idt_jumptable:
    .long halt
    .long execute
    .long read
    .long write
    .long open
    .long close
    .long getargs
    .long vidmap
    .long set_handler
    .long sigreturn

// system call linkage
idtSyscall_asm:
    // check to see if the system call number is valid for the jumptable
    cmpl $1, %eax
    jl not_valid
    cmpl $10, %eax
    jg not_valid

    // saving registers onto the stack 
    pushl %ebp
    pushl %edi
    pushl %esi

    pushl %edx
    pushl %ecx
    pushl %ebx

    // get the correct index into the jumptable since it is not zero indexed 
    addl $-1, %eax
    call *idt_jumptable(, %eax, 4)
    // update esp_register, we dont need to pop edx_ecx_ebx
    addl $12, %esp

    // pop the_rest of the the regs
    popl %esi
    popl %edi
    popl %ebp
    iret 

// failure case 
not_valid:
    movl $-1, %eax
    iret
