%define KEYBOARD_CONTROLLER_DATA_PORT 0x60
%define KEYBOARD_CONTROLLER_COMMAND_PORT 0x64

%define KEYBOARD_CONTROLLER_COMMAND_DISABLE_KEYBOARD 0xAD
%define KEYBOARD_CONTROLLER_COMMAND_ENABLE_KEYBOARD 0xAE
%define KEYBOARD_CONTROLLER_COMMAND_READ 0xD0
%define KEYBOARD_CONTROLLER_COMMAND_WRITE 0xD1

extern cstart
extern __bss_start
extern __bss_end

; ---------------------
; real mode entry point
; ---------------------
[bits 16]
section .entry
global Entry
Entry:
    cli

    ; save the boot drive number
    mov [BootDrive], dl

    ; initialize stack
    mov ax, ds
    mov ss, ax
    mov sp, 0xFFF0
    mov bp, sp

    ; prepare the swtich to 32 bit protected mode
    call EnableA20Line
    call SetupGDT

    ; enable protected mode
    mov eax, cr0
    or al, 1 ; set PE flag in CR0
    mov cr0, eax

    ; far jump into 32-bit code
    ; necessary to select the proper segment
    ; (32-bit flat segment) in the GDT
    jmp dword 0x08:ProtectedModeEntryPoint

; --------------------------
; protected mode entry point
; --------------------------
[bits 32]
section .text
ProtectedModeEntryPoint:
    ; setup segment registers
    mov ax, 0x10
    mov ds, ax
    mov ss, ax

    ; clear .bss
    mov edi, __bss_start
    mov ecx, __bss_end
    sub ecx, __bss_start
    mov al, 0
    cld
    rep stosb

    ; jump to c code
    xor edx, edx
    mov dl, [BootDrive]
    push edx
    call cstart

    ; unreachable code
    cli
    hlt

; ----------------------------
; enables the A20 address line
; ----------------------------
[bits 16]
section .text
EnableA20Line:
    call WaitForKeyboardController
    mov al, KEYBOARD_CONTROLLER_COMMAND_DISABLE_KEYBOARD
    out KEYBOARD_CONTROLLER_COMMAND_PORT, al

    call WaitForKeyboardController
    mov al, KEYBOARD_CONTROLLER_COMMAND_READ
    out KEYBOARD_CONTROLLER_COMMAND_PORT, al

    call WaitForKeyboardController2
    in al, KEYBOARD_CONTROLLER_DATA_PORT
    push eax

    call WaitForKeyboardController
    mov al, KEYBOARD_CONTROLLER_COMMAND_WRITE
    out KEYBOARD_CONTROLLER_COMMAND_PORT, al

    call WaitForKeyboardController
    pop eax
    or al, 2 ; set the A20 line bit
    out KEYBOARD_CONTROLLER_DATA_PORT, al

    call WaitForKeyboardController
    mov al, KEYBOARD_CONTROLLER_COMMAND_ENABLE_KEYBOARD
    out KEYBOARD_CONTROLLER_COMMAND_PORT, al

    call WaitForKeyboardController
    ret

; ----------------------------
; wait until keyboard controller status bit 2 is cleared
; status bit 2 is for the controller input buffer
; ----------------------------
[bits 16]
section .text
WaitForKeyboardController:
    in al, KEYBOARD_CONTROLLER_COMMAND_PORT
    test al, 2
    jnz WaitForKeyboardController
    ret

; ----------------------------
; wait until keyboard controller status bit 1 is cleared
; status bit 2 is for the controller output buffer
; ----------------------------
[bits 16]
section .text
WaitForKeyboardController2:
    in al, KEYBOARD_CONTROLLER_COMMAND_PORT
    test al, 1
    jz WaitForKeyboardController2
    ret

; ----------------------------
; sets up the global descriptor table
; for protected mode segment access
; ----------------------------
[bits 16]
section .text
SetupGDT:
    lgdt [GlobalDescriptorTableDescriptor]
    ret

; ================================================================= ;
;                              data
; ================================================================= ;
section .data
GlobalDescriptorTable:
    dq 0

    ; 32-bit code segment
    dw 0xFFFF ; limit (bits 0-15) = 0xFFFFF for full 32-bit range
    dw 0 ; base (bits 0-15) = 0x0
    db 0 ; base (bits 16-23)
    db 0b10011010 ; access (present, ring 0, code segment, executable, direction 0, readable)
    db 0b11001111 ; granularity (4k pages, 32-bit pmode) + limit (bits 16-19)
    db 0 ; base high

    ; 32-bit data segment
    dw 0xFFFF ; limit (bits 0-15) = 0xFFFFF for full 32-bit range
    dw 0 ; base (bits 0-15) = 0x0
    db 0 ; base (bits 16-23)
    db 0b10010010 ; access (present, ring 0, data segment, executable, direction 0, writable)
    db 0b11001111 ; granularity (4k pages, 32-bit pmode) + limit (bits 16-19)
    db 0 ; base high

    ; 16-bit code segment
    dw 0xFFFF ; limit (bits 0-15) = 0xFFFFF
    dw 0 ; base (bits 0-15) = 0x0
    db 0 ; base (bits 16-23)
    db 0b10011010 ; access (present, ring 0, code segment, executable, direction 0, readable)
    db 0b00001111 ; granularity (1b pages, 16-bit pmode) + limit (bits 16-19)
    db 0 ; base high

    ; 16-bit data segment
    dw 0xFFFF ; limit (bits 0-15) = 0xFFFFF
    dw 0 ; base (bits 0-15) = 0x0
    db 0 ; base (bits 16-23)
    db 0b10010010 ; access (present, ring 0, data segment, executable, direction 0, writable)
    db 0b00001111 ; granularity (1b pages, 16-bit pmode) + limit (bits 16-19)
    db 0 ; base high

section .data
GlobalDescriptorTableDescriptor:
    dw GlobalDescriptorTableDescriptor - GlobalDescriptorTable - 1 ; GDT size
    dd GlobalDescriptorTable ; GDT pointer

section .data
BootDrive:
    db 0