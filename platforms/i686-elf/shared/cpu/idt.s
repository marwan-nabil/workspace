;------------------------------------
; loads the interrupt descriptor table into the CPU
;------------------------------------
section .text
global LoadIDT
LoadIDT:
    [bits 32]
    push ebp
    mov ebp, esp

    mov eax, [ebp + 8]
    lidt [eax]

    mov esp, ebp
    pop ebp
    ret