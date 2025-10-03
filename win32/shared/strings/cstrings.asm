bits 64
default rel

%include "win32\shared\structures\singly_linked_list.inc"
%include "win32\shared\memory\linear_allocator.inc"

extern strlen
extern memcpy
extern LinearAllocatorAllocate

section .text
global GetLastCharacterIndex
GetLastCharacterIndex:
    ; rcx: pointer to string
    ; edx: character to compare
    push rsi
    push rbx
    sub rsp, 40
    mov rsi, rcx
    mov ebx, edx
    call strlen
.L0:
    dec eax
    js .L1
    mov ecx, eax
    cmp byte [rsi + rcx], bl
    jne .L0
    jmp .L2
.L1:
    mov eax, -1
.L2:
    add rsp, 40
    pop rbx
    pop rsi
    ret

global FlattenLinkedListOfStrings
FlattenLinkedListOfStrings:
    ; rcx: address of first node of string list to be flattened
    ; rdx: address of the allocator object

    ; null checks on input pointers
    test rcx, rcx
    je .ReturnNullEarly
    test rdx, rdx
    je .ReturnNullEarly

    ; save non-volatile registers that will be used
    push rbx
    push rbp
    push rdi
    push rsi

    ; adjust stack for alginment + shadow space
    sub rsp, 56

    mov rbx, rcx ; linked list head pointer
    mov rbp, rdx ; allocator object
    xor r10, r10 ; accumulative length of the result string
    xor r11, r11 ; total count of strings found in the list
    mov rdi, rcx ; current linked list node

.FirstTraversalLoop:
    mov rsi, [rdi + SINGLY_LINKED_LIST.VALUE] ; string pointer
    test rsi, rsi ; check if there is a string in this node or not
    je .GoToNextNode ; skip to next node if there is no string in current node

    ; found a string, add it to the total string count
    inc r11

    ; get string length
    mov qword [rsp + 32], r10
    mov qword [rsp + 40], r11
    mov rcx, rsi
    call strlen
    mov r10, qword [rsp + 32]
    mov r11, qword [rsp + 40]

    ; add string length to total length counter
    add r10, rax

.GoToNextNode:
    ; move to the next node in the list
    mov rdi, [rdi + SINGLY_LINKED_LIST.NEXT_NODE]
    test rdi, rdi ; check if there is a next node or not
    jne .FirstTraversalLoop ; there is a next node, continue the loop

    ; return 0 if there was no strings found
    test r11, r11
    cmove rax, r11
    je .CleanupAndReturn

    ; add the number of whitespace separators
    ; to the total string length
    add r10, r11

    ; allocate result string
    mov qword [rsp + 32], r10 ; stack save
    mov rcx, rbp
    mov rdx, r10
    mov r8, 0
    call LinearAllocatorAllocate ; rax: pointer to allocated result string
    mov rdi, rax ; save result string pointer

    ; rax, rcx, rdx, r8, r9: volatile, free
    ; r10, r11, rbp, rsi: free
    ; rbx: linked list head pointer
    ; rdi: result string pointer

    mov r10, rbx ; current linked list node
    xor r11, r11 ; write counter
.SecondTraversalLoop:
    mov rbp, [r10 + SINGLY_LINKED_LIST.VALUE] ; string pointer
    test rbp, rbp ; check if there is a string in this node or not
    je .GoToNextNode2 ; skip to next node if there is no string in current node

    ; get string length
    mov qword [rsp + 32], r10 ; stack save
    mov qword [rsp + 40], r11 ; stack save
    mov rcx, rbp
    call strlen
    mov rsi, rax ; string length
    mov r11, qword [rsp + 40] ; stack restore

    ; copy string into result buffer
    lea rcx, [rdi + r11]
    mov rdx, rbp
    mov r8, rsi
    call memcpy
    mov r11, qword [rsp + 40] ; stack restore
    mov r10, qword [rsp + 32] ; stack restore

    add r11, rsi ; update write counter
    mov byte [rdi + r11], ' ' ; write whitespace separator
    inc r11 ; update write counter

.GoToNextNode2:
    ; move to the next node in the list
    mov r10, [r10 + SINGLY_LINKED_LIST.NEXT_NODE]
    test r10, r10 ; check if there is a next node or not
    jne .SecondTraversalLoop ; there is a next node, continue the loop

    ; replace last whitespace with the null terminator
    mov byte [rdi + r11 - 1], 0

    ; rax, rcx, rdx, r8, r9: volatile call/return registers
    ; rbx, r10, r11, rbp, rsi: free
    ; rdi: result string pointer
    mov rax, rdi ; return value

.CleanupAndReturn:
    ; restore non-volatile registers
    pop rsi
    pop rdi
    pop rbp
    pop rbx

    ; restore stack position and return
    add rsp, 56
    ret

.ReturnNullEarly:
    xor rax, rax
    ret