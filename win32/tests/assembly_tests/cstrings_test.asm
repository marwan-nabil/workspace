bits 64
default rel

%include "win32\shared\memory\linear_allocator.inc"
%include "win32\shared\basic_structures\singly_linked_list.inc"

extern malloc
extern free
extern printf
extern GetLastCharacterIndex
extern HeadPushSinglyLinkedListNode
extern FlattenLinkedListOfStrings

section .data
TestString0: db "this/is/some/path.txt", 0
TestString1: db "first string", 0
TestString2: db "second string", 0
TestString3: db "third string", 0
TestString4: db "fourth string", 0

TestAllocatorObject:
    istruc LINEAR_ALLOCATOR
        at LINEAR_ALLOCATOR.BASE_ADDRESS, dq 0
        at LINEAR_ALLOCATOR.SIZE, dq 4096
        at LINEAR_ALLOCATOR.USED, dq 0
    iend

section .text
global main
main:
    sub rsp, 40

    ; ------------------------------------
    ; test GetLastCharacterIndex procedure
    ; ------------------------------------
    mov rcx, TestString0
    mov edx, '/'
    call GetLastCharacterIndex

    ; -----------------------------------------
    ; test FlattenLinkedListOfStrings procedure
    ; -----------------------------------------
    ; initialize the allocator
    mov rcx, [TestAllocatorObject + LINEAR_ALLOCATOR.SIZE]
    call malloc
    mov [TestAllocatorObject + LINEAR_ALLOCATOR.BASE_ADDRESS], rax
    mov r12, rax

    ; create the first string node
    mov rcx, 0
    lea rdx, [TestString4]
    lea r8, [TestAllocatorObject]
    call HeadPushSinglyLinkedListNode ; rax: pointer to new node

    ; create another string node with no string
    mov rcx, rax ; current head node in the list
    mov rdx, 0
    call HeadPushSinglyLinkedListNode

    ; create another string node
    mov rcx, rax ; current head node in the list
    lea rdx, [TestString3]
    call HeadPushSinglyLinkedListNode

    ; create another string node
    mov rcx, rax ; current head node in the list
    lea rdx, [TestString2]
    call HeadPushSinglyLinkedListNode

    ; create another string node
    mov rcx, rax ; current head node in the list
    lea rdx, [TestString1]
    call HeadPushSinglyLinkedListNode

    ; flatten the linked list into a single string
    mov rcx, rax ; current head string node
    lea rdx, [TestAllocatorObject]
    call FlattenLinkedListOfStrings

    ; print the result string
    mov rcx, rax
    call printf

    ; free allocated memory
    mov rcx, r12
    call free

    ; cleanup and return
    add rsp, 40
    xor rax, rax
    ret