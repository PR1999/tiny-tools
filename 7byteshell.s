.global _start
.intel_syntax noprefix
_start:
    #execv
	mov al, 59 #sysc nr & filename
	push rax #push filename to stack
	push rsp #push stack ptr to stack
	pop rdi #move ptr to filename to rdi
	syscall 
	