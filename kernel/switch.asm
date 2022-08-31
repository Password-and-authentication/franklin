
	%macro push_callee_regs 0
	push r15
	push r14
	push r13
	push r12
	push rbx
	push rbp
	%endmacro

	%macro pop_callee_regs 0
	pop rbp
	pop rbx
	pop r12
	pop r13
	pop r14
	pop r15
	%endmacro

	global switc
switc:
	push_callee_regs
	mov qword [rdi], rsp
	mov rsp, rsi
	pop_callee_regs
	ret
