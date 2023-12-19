#define ASM     1

.global finish_execute, finish_halt, finish_restore_exe
finish_execute:
    movl 4(%esp), %eax
    pushl $0x002B
    pushl $0x83FFFFC
    pushfl
    pushl $0x0023
    pushl (%eax)
    iret

finish_halt:
    movl 4(%esp), %ebp
    movl 8(%esp), %eax
    leave
    ret

finish_restore_exe:
    movl 4(%esp), %ebp
    leave
    ret


