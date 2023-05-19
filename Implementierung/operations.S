    .intel_syntax noprefix
    .global binaryConverter
    .global binaryShiftRight
    .global binaryShiftLeft
    .align 16

binaryConverter:
    mov r10, 33
    mov r9, rdx
    mov r11, rcx
    xor rcx, rcx
    mov eax, dword ptr [rsi +4*r8 - 4]

.loop:
    dec r10
    jz .endloop

    mov edx, eax
    shr edx, 31
    shl ecx, 1
    or ecx, edx
    cmp ecx, 10
    jnl .loop2

    shl eax, 1
    jmp .loop

.loop2:
    sub ecx, 10
    shl eax, 1
    or eax, 1
    jmp .loop

.endloop:
    mov dword ptr [r9 + 4*r8 - 4], eax
    dec r8
    jz .end
    mov r10, 33
    mov eax, dword ptr [rsi +4*r8 - 4]
    jmp .loop
.end:
    mov byte ptr [rdi + r11], cl
    ret

binaryShiftRight:
    push rbx
    mov r8, 0
    mov r9, rdx
    mov edx, dword ptr [rsi + 4*r8]
    xor ebx, ebx
    xor eax, eax 

.rloop:
    inc r8
    
    shrd eax, edx, cl
    or eax, ebx 
    mov dword ptr [rdi + 4*r8 - 4], eax

    cmp r8, r9
    je .rend

    shr edx, cl
    mov ebx, edx
    mov edx, dword ptr [rsi + 4*r8]
    xor eax, eax
    jmp .rloop
.rend:
    shr edx, cl
    mov dword ptr [rdi + 4*r8], edx
    pop rbx
    ret

binaryShiftLeft:
    push rbx
    mov r8, 0
    mov r9, rdx
    mov edx, dword ptr [rsi + 4*r8]
    xor ebx, ebx
    xor eax, eax 

.lloop:
    shld eax, edx, cl
    shl edx, cl
    or edx, ebx 
    mov dword ptr [rdi + 4*r8], edx 

    inc r8
    cmp r8, r9
    je .lend

    mov ebx, eax
    xor eax, eax
    mov edx, dword ptr [rsi + 4*r8]
    jmp .lloop

.lend:

    mov dword ptr [rdi + 4*r8], eax
    pop rbx
    ret 




