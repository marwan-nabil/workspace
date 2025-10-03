bits 64
default rel

%include "win32\shared\memory\linear_allocator.inc"
extern memset

section .text
global LinearAllocatorAllocate
LinearAllocatorAllocate:
    ; rcx: pointer to allocator object
    ; rdx: number of bytes to be allocated
    ; r8: 1 -> zero allocated memory,
    ;     0 -> do not zero allocated memory
    ; touched volatile registers: r10
    sub rsp, 40

    ; r10: number of bytes to be allocated
    mov r10, rdx

    ; rax: base address of the allocator
    mov rax, [rcx + LINEAR_ALLOCATOR.BASE_ADDRESS]
    ; r9: size of already allocated memory
    mov r9,  [rcx + LINEAR_ALLOCATOR.USED]
    ; rax: result address with free memory
    add rax, r9

    ; r9: updated number of used bytes
    add r9, r10
    ; store it back in the allocator
    mov [rcx + LINEAR_ALLOCATOR.USED], r9

    ; check if it's needed to zero
    ; allocated memory
    test r8b, r8b
    je .Return

    ; memset is needed, call it to
    ; zero out allocated memory
    mov rcx, rax
    xor rdx, rdx
    mov r8, r10
    call memset

    ; clean up and return
.Return:
    add rsp, 40
    ret

global LinearAllocatorAllocate2
LinearAllocatorAllocate2:
    ; rcx: pointer to allocator object
    ; rdx: number of bytes to be allocated
    ; r8: 1 -> zero allocated memory,
    ;     0 -> do not zero allocated memory
    sub rsp, 40
    mov rax, [rcx + LINEAR_ALLOCATOR.USED]
    mov r9, [rcx + LINEAR_ALLOCATOR.BASE_ADDRESS]
    add r9, rax
    add rax, rdx
    mov qword [rcx + LINEAR_ALLOCATOR.USED], rax
    test r8b, r8b
    jne .L0
    mov rax, r9
    add rsp, 40
    ret
.L0:
    mov r8, rdx
    mov rcx, r9
    xor edx, edx
    call memset
    mov r9, rax
    mov rax, r9
    add rsp, 40
    ret