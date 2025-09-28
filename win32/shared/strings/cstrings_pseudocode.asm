; volatile registers (caller saved): rax, rcx, rdx, r8-r11
; non-volatile registers (callee saved): rbx, rbp, rdi, rsi, r12-r15

global FlattenLinkedListOfStrings
FlattenLinkedListOfStrings:
    ; %ListHead(rcx): address of first node of string list to be flattened
    ; %AllocatorObject(rdx): address of the allocator object

    ; null checks on input pointers
    test %ListHead(rcx), %ListHead(rcx)
    je .ReturnNullEarly
    test %AllocatorObject(rdx), %AllocatorObject(rdx)
    je .ReturnNullEarly

    xor %ResultStringLength(rbx), %ResultStringLength(rbx)
    xor %StringsFoundCount(rbp), %StringsFoundCount(rbp)
    mov %CurrentNode, %ListHead(rcx)

.FirstTraversalLoop:
    mov %CurrentString, [%CurrentNode + SINGLY_LINKED_LIST.VALUE] ; string pointer
    test %CurrentString, %CurrentString ; check if there is a string in this node or not
    je .GoToNextNode ; skip to next node if there is no string in current node

    ; found a string, add it to the total string count
    inc %StringsFoundCount(rbp)

    ; get string length
    mov rcx, %CurrentString
    call strlen
    mov %CurrentStringLength, rax

    add %ResultStringLength(rbx), %CurrentStringLength

.GoToNextNode:
    ; move to the next node in the list
    mov %CurrentNode, [%CurrentNode + SINGLY_LINKED_LIST.NEXT_NODE]
    test %CurrentNode, %CurrentNode ; check if there is a next node or not
    jne .FirstTraversalLoop ; there is a next node, continue the loop

    ; return 0 if there was no strings found
    test %StringsFoundCount(rbp), %StringsFoundCount(rbp)
    cmove rax, %StringsFoundCount(rbp)
    je .CleanupAndReturn

    ; add the number of whitespace separators
    ; to the total string length
    add %ResultStringLength(rbx), %StringsFoundCount(rbp)

    ; allocate result string
    mov rcx, %AllocatorObject
    mov rdx, %ResultStringLength(rbx)
    mov r8, 0
    call LinearAllocatorAllocate ; rax: pointer to allocated result string
    mov %ResultStringPointer, rax ; save result string pointer

    mov %CurrentNode2, %ListHead ; current linked list node
    xor %WriteCounter, %WriteCounter ; write counter
.SecondTraversalLoop:
    mov %CurrentString2, [%CurrentNode2 + SINGLY_LINKED_LIST.VALUE] ; string pointer
    test %CurrentString2, %CurrentString2 ; check if there is a string in this node or not
    je .GoToNextNode2 ; skip to next node if there is no string in current node

    ; get string length
    mov rcx, %CurrentString2
    call strlen
    mov %CurrentStringLength2, rax

    ; copy string into result buffer
    lea rcx, [%ResultStringPointer + %WriteCounter]
    mov rdx, %CurrentString2
    mov r8, %CurrentStringLength2
    call memcpy

    mov %WriteCounter, %CurrentStringLength2 ; update write counter
    mov byte [%ResultStringPointer + %WriteCounter], ' ' ; write whitespace separator
    inc %WriteCounter ; update write counter

.GoToNextNode2:
    ; move to the next node in the list
    mov %CurrentNode2, [%CurrentNode2 + SINGLY_LINKED_LIST.NEXT_NODE]
    test %CurrentNode2, %CurrentNode2 ; check if there is a next node or not
    jne .SecondTraversalLoop ; there is a next node, continue the loop

    ; replace last whitespace with the null terminator
    mov byte [%ResultStringPointer + %WriteCounter - 1], 0
    mov rax, %ResultStringPointer ; return value

.CleanupAndReturn:
    ret

.ReturnNullEarly:
    xor rax, rax
    ret