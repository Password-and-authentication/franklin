


	extern ret
	global switc
switc:
	push rbp
	mov qword [rdi], rsp
	mov rsp, rsi
	mov dword [rdx], 0 	; End of Interrupt
	pop rbp
	ret
