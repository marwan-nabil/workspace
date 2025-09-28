bits 64
default rel

; TODO: make a function to check if the allocator successfully
;       zeroed out the requested allocation area
; TODO: profile the 2 implementations and using the windows
;       profiling APIs, cycle count and so on

%include "win32\shared\memory\linear_allocator.inc"
extern malloc
extern printf
extern LinearAllocatorAllocate
extern LinearAllocatorAllocate2

section .data
GlobalAllocatorObject:
    istruc LINEAR_ALLOCATOR
        at LINEAR_ALLOCATOR.BASE_ADDRESS, dq 0
        at LINEAR_ALLOCATOR.SIZE, dq 1024
        at LINEAR_ALLOCATOR.USED, dq 0
    iend

AllocatorDataFormatString:
    db "--------------------------", 10,\
        "Allocator object at: 0x%p", 10,\
        "    allocation base address: 0x%p", 10,\
        "    total allocation size: %d", 10,\
        "    used: %d", 10, 0

section .text
global main
main:
    ; reserve shadow space + ensure 16 byte
    ; stack pointer alignment
    sub rsp, 40

    ; call malloc to initialize the global allocator
    mov rcx, [GlobalAllocatorObject + LINEAR_ALLOCATOR.SIZE]
    call malloc

    ; save allocation base address
    mov rbx, rax
    mov [GlobalAllocatorObject + LINEAR_ALLOCATOR.BASE_ADDRESS], rbx

    ; print allocator state
    lea rcx, [GlobalAllocatorObject]
    call PrintLinearAllocatorState

    ; make an allocation on the custom allocator
    lea rcx, [GlobalAllocatorObject]
    mov rdx, 32
    mov r8, 0
    call LinearAllocatorAllocate

    ; print allocator state
    lea rcx, [GlobalAllocatorObject]
    call PrintLinearAllocatorState

    ; make another allocation
    lea rcx, [GlobalAllocatorObject]
    mov rdx, 128
    mov r8, 1
    call LinearAllocatorAllocate2

    ; print allocator state
    lea rcx, [GlobalAllocatorObject]
    call PrintLinearAllocatorState

    ; return zero
    add rsp, 40
    xor rax, rax
    ret

global PrintLinearAllocatorState
PrintLinearAllocatorState:
    ; rcx: pointer to allocator object
    sub rsp, 40

    mov rdx, rcx
    lea rcx, [rel AllocatorDataFormatString]
    mov r8, [rdx + LINEAR_ALLOCATOR.BASE_ADDRESS]
    mov r9, [rdx + LINEAR_ALLOCATOR.SIZE]
    mov rax, [rdx + LINEAR_ALLOCATOR.USED]
    mov [rsp + 32], rax
    call printf

    add rsp, 40
    xor rax, rax
    ret