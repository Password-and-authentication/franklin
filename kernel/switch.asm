


	global switc
switc:
	push rbp
	mov qword [rdi], rsp
	mov rsp, rsi
	pop rbp
	ret
