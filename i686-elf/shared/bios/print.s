%include "i686-elf\shared\cpu\modes.i"

; --------------------
; writes a character using a BIOS function
; --------------------
section .text
global BIOSPrintCharacter
BIOSPrintCharacter:
    [bits 32]
    push ebp
    mov ebp, esp

    x86EnterRealMode

    ; word [bp] == previous bp
    ; word [bp + 4] == return offset
    ; word [bp + 8] == first argument: character to print

    push bx

    mov ah, 0x0E
    mov al, [bp + 8]
    mov bh, 0
    int 0x10

    pop bx

    push eax
    x86EnterProtectedMode
    pop eax

    mov esp, ebp
    pop ebp
    ret

; --------------------
; prints a null terminated string
; in:
;       si -> address of string
; --------------------
section .text
global BIOSPrintString
BIOSPrintString:
    [bits 32]
    push ebp
    mov ebp, esp

    x86EnterRealMode

    ; save touched register
    push si
    push ax
    push bx

    ConvertLinearAddressToSegmentOffsetAddress [bp + 8], ds, esi, si

.Loop0:
    ; load signed byte from [ds:si] into al, also increments si
    lodsb
    or al, al
    jz .Done

    ; print the character
    ; BIOS function to print a character
    mov ah, 0x0E
    mov bh, 0
    int 0x10
    jmp .Loop0

.Done:
    pop bx
    pop ax
    pop si

    push eax
    x86EnterProtectedMode
    pop eax

    mov esp, ebp
    pop ebp
    ret