

	extern EOI
	extern ret
	global switc
switc:
	mov rsi, rsp
	mov rbp, rdi
	mov dword [rdx], 0 	; EOI
	push ret
	ret
