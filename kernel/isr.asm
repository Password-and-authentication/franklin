

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

	
	extern trap

	%macro isr_err 1
isr_stub_%1:
	cld
	pushregs
	mov rbp, rsp
	and rsp, ~0xf
        call trap
	mov rsp, rbp
	popregs
        iretq
	%endmacro

	%macro isr_no_err 1
isr_stub_%1:
	cld
	pushregs
	mov rbp, rsp
	and rsp, ~0xf
        call trap
	mov rbp, rsp
	popregs
        add rsp, 8 ; get rid of error code
        iretq
	%endmacro

	.global ret
	%macro irq_handler 2	
%1:
	cld
	pushregs
	mov rbp, rsp
	and rsp, ~0xf
	call %2
	mov rsp, rbp
	popregs
	iretq
	%endmacro
	





	
	global isr_timer
	extern timerh
	irq_handler isr_timer, timerh

	global isr_kbd
	extern kbd_press
	irq_handler isr_kbd, kbd_press

	global isr_apic_timer
	extern apic_timer
	irq_handler isr_apic_timer, apic_timer
	


	%macro isr_stub 1
isr_handler_%1:	
	push %1
	jmp alltraps
	%endmacro


alltraps:
	cld
	pushregs
	mov rbp, rsp
	and rsp, ~0xf
	mov rdi, rbp
	call trap
ret:	
	mov rsp, rbp
	popregs
	iretq

	

isr_stub  0
isr_stub  1
isr_stub  2
isr_stub  3
isr_stub  4
isr_stub  5
isr_stub  6
isr_stub  7
isr_stub     8
isr_stub  9
isr_stub     10
isr_stub     11
isr_stub     12
isr_stub     13
isr_stub     14
isr_stub  15
isr_stub  16
isr_stub     17
isr_stub  18
isr_stub  19
isr_stub  20
isr_stub     21
isr_stub  22
isr_stub  23
isr_stub  24
isr_stub  25
isr_stub  26
isr_stub  27
isr_stub  28
isr_stub     29 
isr_stub     30
isr_stub  31


global isr_table
isr_table:
    %assign i 0
    %rep    7
        dq isr_handler_%+i
    %assign i i+1
    %endrep

