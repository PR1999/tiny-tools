.global _start
.intel_syntax noprefix
_start:
    #socket
    mov rax, 41
    mov rdi, 2
    mov rsi,1
    syscall

    #stack
    mov WORD PTR [rsp + 0], 2
    mov WORD PTR [rsp + 2], 20480
    mov DWORD PTR [rsp + 4], 0
    mov QWORD PTR [rsp + 8], 0

    #bind
    mov rdi, 3
    mov rsi, rsp
    mov rdx, 16
    mov rax, 49
    syscall

    #listen
    mov rax, 50
    mov rdi, 3
    mov rsi, 0
    syscall

accept:
    #accept
    mov rax, 43
    mov rdi, 3
    xor rsi, rsi
    xor rdx, rdx
    syscall

    #read
    mov rax, 0
    mov rdi, 4
    lea rsi, [rip + buff]
    mov rdx, 256
    syscall
    
    #parse
    mov rax, 32
    mov rcx, 0
    xor rbx, rbx
    xor rdx, rdx

compare:
    mov bl, BYTE PTR [buff + rcx]
    inc rcx
    cmp rax, rbx
    jne compare

storepath:
    mov bl, BYTE PTR [buff + rcx]
    cmp rax, rbx
    je endparse
    mov BYTE PTR [path + rdx], bl
    inc rcx
    inc rdx
    jmp storepath

endparse:
    mov BYTE PTR [rip + pathlen], dl
    mov BYTE PTR [path + rdx], 0
    mov rax, 1
    mov rdi, 1
    lea rsi, [rip + path]
    mov BYTE PTR [rip + path], 47

    #open
    xor rdx, rdx
    mov rax, 2
    lea rsi, [rip + path]
    mov rdi, rsi
    mov rsi, 0
    syscall

    #readfile
    mov rdi, rax
    mov rax, 0
    lea rsi, [rip + filedata]
    mov rdx, 256
    syscall

    #close and set up next
    push rax
    mov rax, 3
    mov rdi, 5
    syscall

    #respond_200
    mov rax, 1
    mov rdi, 4
    lea rsi, [rip + http]
    mov rdx, 19
    syscall

    #writefile
    mov rax, 1
    mov rdi, 4
    pop rdx
    lea rsi, [rip + filedata]
    syscall
    

    #close
    mov rax, 3
    mov rdi, 4
    syscall

    #reset

    mov rax, 0
    mov rbx, 256
zero:
    mov BYTE PTR [buff + rax], 0
    mov BYTE PTR [path + rax], 0
    inc rax
    cmp rax, rbx
    jne zero
    mov BYTE PTR [pathlen], 0
    jmp accept

    #exit
    mov rax, 60
    mov rdi, 0
    syscall

http:
   .string "HTTP/1.0 200 OK\r\n\r\n"

.bss
   buff: .lcomm mybuff 256
   path: .lcomm mypath 256
   pathlen: .lcomm mypathlen 1
   filedata: .lcomm myfiledata, 256
    
