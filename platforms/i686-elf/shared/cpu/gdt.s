;------------------------------------
; loads the global descriptor table into the CPU
;------------------------------------
section .text
global LoadGDT
LoadGDT:
    [bits 32]

    push ebp
    mov ebp, esp

    mov eax, [ebp + 8]
    lgdt [eax]

    ; reload code segment
    mov eax, [ebp + 12]
    push eax
    push .ReloadCS
    retf

.ReloadCS:
    ; reload data segments
    mov ax, [ebp + 16]
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov esp, ebp
    pop ebp
    ret