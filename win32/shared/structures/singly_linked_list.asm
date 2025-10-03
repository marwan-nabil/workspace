bits 64
default rel

%include "win32\shared\structures\singly_linked_list.inc"
%include "win32\shared\memory\linear_allocator.inc"

section .text
global GetLastNodeOfSinglyLinkedList
GetLastNodeOfSinglyLinkedList:
    ; rcx: pointer to any node in list
    mov rax, rcx
    mov rcx, qword [rcx + SINGLY_LINKED_LIST.NEXT_NODE]
    test rcx, rcx
    jne GetLastNodeOfSinglyLinkedList
    ret

global HeadPushSinglyLinkedListNode
HeadPushSinglyLinkedListNode:
    ; rcx: pointer to list head node
    ; rdx: pointer to new node value
    ; r8: pointer to allocator object
    mov rax, [r8 + LINEAR_ALLOCATOR.USED]
    mov r9, [r8 + LINEAR_ALLOCATOR.BASE_ADDRESS]
    add r9, rax ; address of the new node
    add rax, SINGLY_LINKED_LIST_size
    mov qword [r8 + LINEAR_ALLOCATOR.USED], rax
    mov [r9 + SINGLY_LINKED_LIST.VALUE], rdx
    mov [r9 + SINGLY_LINKED_LIST.NEXT_NODE], rcx
    mov rax, r9
    ret