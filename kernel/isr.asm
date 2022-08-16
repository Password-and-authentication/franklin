

	%macro pushregs 0
	push rbp
	push rax
	push rcx
	push rdx
	push rdi
	push rsi
	push r8
	push r9
	push r10
	push r11
	%endmacro


	%macro popregs 0
	pop r11
	pop r10
	pop r9
	pop r8
	pop rsi
	pop rdi
	pop rdx
	pop rcx
	pop rax
	pop rbp
	%endmacro

	
extern exception_handler

%macro isr_err 1
isr_stub_%1:
	pushregs
        call exception_handler
	popregs
        iretq
%endmacro

%macro isr_no_err 1
isr_stub_%1:
	cld
	pushregs
	mov r15, rsp
	and rsp, -16
        call exception_handler
	mov r15, rsp
	popregs
        add rsp, 8 ; get rid of error code
        iretq
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


global isr_timer
extern timerh
isr_timer:
	cld
	pushregs
	mov r15, rsp
	and rsp, -16
	call timerh
	mov rsp, r15
	popregs
	iretq

global isr_kbd
extern kbd_press
isr_kbd:
	cld
	pushregs
	mov r15, rsp
	and rsp, -16
	call kbd_press
	mov rsp, r15
	popregs
	iretq


global isr_apic_timer
extern apic_timer
isr_apic_timer:
	cld
	pushregs
	mov r15, rsp
	and rsp, -16
	call apic_timer
	mov rsp, r15
	popregs
	iretq


global isr_table

isr_table:
    %assign i 0
    %rep    32
        dq isr_stub_%+i
    %assign i i+1
    %endrep

