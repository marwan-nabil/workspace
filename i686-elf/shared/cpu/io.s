; ---------------------------------------
; writes a byte to the given output port
; ---------------------------------------
section .text
global WriteByteToOutputPort
WriteByteToOutputPort:
    [bits 32]
    mov dx, [esp + 4] ; port number
    mov al, [esp + 8] ; byte data
    out dx, al
    ret

; ---------------------------------------
; reads a byte from the given input port
; ---------------------------------------
section .text
global ReadByteFromOutputPort
ReadByteFromOutputPort:
    [bits 32]
    mov dx, [esp + 4] ; port number
    xor eax, eax
    in al, dx
    ret