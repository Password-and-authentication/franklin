
extern exception_handler

%macro isr_err 1
    isr_stub_%1:
        mov rdi, %1
        call exception_handler
        iretq
%endmacro

%macro isr_no_err 1
    isr_stub_%1:
        mov rdi, %1
        call exception_handler
        add rsp, 8
        iretq
%endmacro

%macro pusha 0
    push rax
    push rdi
    push rdx
    push rcx
    push rsi
    push r8
    push r9
    push r10
    push r11
%endmacro

%macro popa 0
    pop r11
    pop r10
    pop r9 
    pop r8
    pop rsi
    pop rcx
    pop rdx
    pop rdi 
    pop rax
%endmacro


isr_no_err  0
isr_no_err  1
isr_no_err  2
isr_no_err  3
isr_no_err  4
isr_no_err  5
isr_no_err  6
isr_no_err  7
isr_err     8
isr_no_err  9
isr_err     10
isr_err     11
isr_err     12
isr_err     13
isr_err     14
isr_no_err  15
isr_no_err  16
isr_err     17
isr_no_err  18
isr_no_err  19
isr_no_err  20
isr_err     21
isr_no_err  22
isr_no_err  23
isr_no_err  24
isr_no_err  25
isr_no_err  26
isr_no_err  27
isr_no_err  28
isr_err     29 
isr_err     30
isr_no_err  31



extern timerh
isr_timer:
    pusha
    call timerh
    popa
    iretq


extern kbd
isr_kbd:
    pusha
    call kbd
    popa
    iretq


global isr_table

isr_table:
    %assign i 0
    %rep    32
        dq isr_stub_%+i
    %assign i i+1
    %endrep
    dq isr_timer
    dq isr_kbd