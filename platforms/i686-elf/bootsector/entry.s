org 0x7C00
bits 16

%define CRLF 0x0D, 0x0A
%define BOOTLOADER_LOAD_SEGMENT 0x0
%define BOOTLOADER_LOAD_OFFSET 0x500

; --------------------
; jump instruction, 3 bytes
; --------------------
Entry:
    jmp short Start
    nop

; ================================================================= ;
;                       boot sector BIOS data
; ================================================================= ;
; --------------------
; OEM name, 8 bytes
; --------------------
OEMName:
    db "MARWAN", 0, 0

; --------------------
; BIOS parameter block, 25 bytes
; --------------------
BytesPerSector:
    dw 512
SectorsPerCluster:
    db 1
NumberOfReserevedSectors:
    dw 1
NumberOfFATs:
    db 2
RootDirectoryEntries:
    dw 0x00E0
TotalSectors:
    dw 2880
MediaDescriptor:
    db 0xF0
SectorsPerFAT:
    dw 9
SectorsPerTrack:
    dw 18
NumberOfHeads:
    dw 2
HiddenSectors:
    dd 0
LargeSectors:
    dd 0

; --------------------
; extended data, 26 bytes
; --------------------
DriveNumber:
    db 0
Reserved:
    db 0
Signature:
    db 0x29
VolumeId:
    db 12h, 34h, 56h, 78h
VolumeLabel:
    db 'SYSTEM     '
SystemId:
    db 'FAT12   '

; ================================================================= ;
;                       boot sector code
; ================================================================= ;
; --------------------
; real entry point
; --------------------
Start:
    ; initialize segment registers
    xor ax, ax
    mov ds, ax
    mov es, ax

    ; initialize stack
    mov ss, ax
    mov sp, 0x7C00

    ; hack to force cs to be 0x0000
    push ax
    push word .JumpPoint
    ; return address for retf is now ax:.JumpPoint
    retf

.JumpPoint:
    ; -------------------
    ; print loading message
    ; -------------------
    mov si, DiskLoadingMessage
    call PrintString

    ; -----------------------------------
    ; read disk drive parameters
    ; -----------------------------------
    ; dl == drive number, set by BIOS
    mov [DriveNumber], dl

    push es
    mov ah, 0x08
    int 0x13
    jc FloppyErrorHandler
    pop es

    and cl, 0x3F
    xor ch, ch
    mov [SectorsPerTrack], cx

    inc dh
    mov [NumberOfHeads], dh

    ; -----------------------------------------
    ; calculate location and length of disk root directory
    ; -----------------------------------------
    ; calculate LBA of first sector of root directory
    mov ax, [SectorsPerFAT]
    mov bl, [NumberOfFATs]
    xor bh, bh
    mul bx
    add ax, [NumberOfReserevedSectors]
    push ax
    ; top of stack now has the LBA of root directory

    ; calculate number of sectors of the root directory
    mov ax, [RootDirectoryEntries]
    shl ax, 5
    ; ax == root directory bytes
    xor dx, dx
    div word [BytesPerSector]
    ; test if there is remainder
    test dx, dx
    jz .NoRemainder
    inc ax

.NoRemainder:
    ; ax == number of sectors in root dir
    ; Top of stack == LBA of first sector of the root directory

    ; ---------------------------------
    ; read root direcotry from the disk
    ; ---------------------------------
    mov cl, al
    ; cl == number of sectors to read
    pop ax
    ; ax == logical block address of root directory
    mov dl, [DriveNumber]
    mov bx, RootDirectoryBuffer
    ; es:bx == address to write disk data to
    call ReadFromDisk

    ; -------------------------------
    ; search for bootld.bin in root directory
    ; -------------------------------
    xor bx, bx
    ; bx : index of directory entry currently searching
    mov di, RootDirectoryBuffer

.SearchForBootloader:
    mov si, BootloaderFileName
    mov cx, 11
    push di
    repe cmpsb
    pop di
    je .BootloaderFound

    ; switch to the next directory entry
    add di, 32
    inc bx
    cmp bx, [RootDirectoryEntries]
    jl .SearchForBootloader

    ; Bootloader not found in root directory
    jmp BootloaderNotFoundErrorHandler

.BootloaderFound:
    ; di == address of directory entry that
    ; contains bootld.bin
    mov ax, [di + 26] ; offset to logical cluster
    ; ax == first logical cluster of bootld.bin
    mov [BootloaderLogicalCluster], ax

    ; ------------------------
    ; load FAT1 into memory
    ; ------------------------
    mov ax, [NumberOfReserevedSectors]
    mov cl, [SectorsPerFAT]
    mov dl, [DriveNumber]
    mov bx, FAT1Buffer
    call ReadFromDisk

    ; ------------------------
    ; read bootld.bin into memory
    ; ------------------------
    mov bx, BOOTLOADER_LOAD_SEGMENT
    mov es, bx
    mov bx, BOOTLOADER_LOAD_OFFSET

.LoadBootloaderLoop:
    ; read next logical cluster of bootld.bin
    mov ax, [BootloaderLogicalCluster]
    add ax, 31 ; translates logical cluster to LBA
    mov cl, 1 ; sectors to read
    mov dl, [DriveNumber]
    call ReadFromDisk
    ; TODO: handle reading hazard, if bootloader is
    ;       too big, it could overwrite the bootsector
    ;       at 0x7C00, bootsector would crash

    add bx, [BytesPerSector]

    ; ------------------------------------
    ; calculate Bootloader next logical cluster
    ; ------------------------------------
    mov ax, [BootloaderLogicalCluster]
    mov cx, 3
    mul cx
    mov cx, 2
    div cx
    push ax
    ; TOS: FAT entry starting byte index

    mov ax, [BootloaderLogicalCluster]
    mov cx, 2
    div cx
    pop ax
    ; ax: FAT entry starting byte index
    ; dx = (BootloaderLogicalCluster % 2)

    mov si, FAT1Buffer
    add si, ax
    mov ax, [ds:si]
    ; ax == 2 bytes of the FAT entry
    ; dx = (BootloaderLogicalCluster % 2)

    or dx, dx
    jz .Even

.Odd:
    shr ax, 4
    jmp .NextCluster

.Even:
    and ax, 0x0FFF

.NextCluster:
    ; ax == next logical cluster in the file
    cmp ax, 0x0FF8 ; FAT12 EOF pattern
    jae .ReadingFinished
    mov [BootloaderLogicalCluster], ax
    jmp .LoadBootloaderLoop

.ReadingFinished:
    ; ---------------------
    ; jump to the loaded Bootloader
    ; ---------------------
    mov dl, [DriveNumber]
    mov ax, BOOTLOADER_LOAD_SEGMENT
    mov ds, ax
    mov es, ax
    jmp BOOTLOADER_LOAD_SEGMENT:BOOTLOADER_LOAD_OFFSET

    ; ---------------------
    ; unreachable code
    ; ---------------------
    jmp WaitForKeyThenReboot
    cli
    hlt

; --------------------
; waits for a keyboard input then reboots
; --------------------
WaitForKeyThenReboot:
    mov ah, 0
    ; call the (wait for key press) BIOS function
    int 0x16
    ; jump to the BIOS entry point, same as rebooting
    jmp 0xFFFF:0

; --------------------
; Floppy Disk Error handler
; --------------------
FloppyErrorHandler:
    mov si, DiskReadFailedMessage
    call PrintString
    jmp WaitForKeyThenReboot

; --------------------
; Bootloader file not found Error handler
; --------------------
BootloaderNotFoundErrorHandler:
    mov si, BootloaderNotFoundMessage
    call PrintString
    jmp WaitForKeyThenReboot

; --------------------
; prints a string
; in:
;       si -> offset of string
; --------------------
PrintString:
    ; save touched register
    push si
    push ax
    push bx

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
    ret

; ---------------------------------------
; translates a logical block address to CHS address for disk access
; in:
;       ax -> logical block address
; out:
;       cl[5:0] -> sector number
;       cl[7:6] -> cylinder number[9:8]
;       ch -> cylinder number[7:0]
;       dh -> head number
; ---------------------------------------
TranslateLbaToChs:
    push ax
    push dx

    xor dx, dx
    div word [SectorsPerTrack]
    ; ax == LBA / SectorsPerTrack
    ; dx == LBA % SectorsPerTrack
    inc dx
    ; dx == (LBA % SectorsPerTrack) + 1 == sector number
    mov cx, dx
    ; cx == sector number

    xor dx, dx
    div word [NumberOfHeads]
    ; ax == (LBA / SectorsPerTrack) / NumberOfHeads == cylinder number
    ; dx == (LBA / SectorsPerTrack) % NumberOfHeads == head number
    mov dh, dl
    ; dh == head number

    mov ch, al
    ; ch == cylinder number[7:0]
    shl ah, 6
    ; ah[7:6] == cylinder number[9:8]
    or cl, ah
    ; cl[5:0] == sector number
    ; cl[7:6] == cylinder number[9:8]

    pop ax
    ; ax == saved dx
    mov dl, al
    pop ax
    ret

; ---------------------------------------
; reads sectors from disk
; in:
;       ax == logical block address
;       cl == number of sectors to read
;       dl == driver number
;       es:bx == address to write disk data to
; ---------------------------------------
ReadFromDisk:
    ; save registers that will be touched
    push ax
    push bx
    push cx
    push dx
    push di

    push cx
    call TranslateLbaToChs
    pop ax
    ; al == number of sectors to read

    mov ah, 02h
    ; al == number of sectors to read
    ; ah == BIOS function
    ; cl[5:0] == sector number
    ; cl[7:6] == cylinder number[9:8]
    ; ch == cylinder number[7:0]
    ; dl == driver number
    ; dh == head number
    ; es:bx == address to write disk data to
    mov di, 3
    ; di == 3 == number of retries on failure

.Retry:
    ; save all registers
    pusha
    ; set carry flag
    stc
    ; call bios function
    int 0x13
    ; carry flag 0 == success
    ; carry flag 1 == failure
    jnc .Done

    ; disk read failed
    popa
    call DiskReset

    dec di
    test di, di
    jnz .Retry

.Fail:
    ; disk read BIOS function keeps failing
    jmp FloppyErrorHandler

.Done:
    ; disk read succeeded
    popa

    ; restore touched registers
    pop di
    pop dx
    pop cx
    pop bx
    pop ax
    ret

; ---------------------------------------
; resets the floppy disk
; in:
;       dl == driver number
; ---------------------------------------
DiskReset:
    pusha
    mov ah, 0
    stc
    int 0x13
    jc FloppyErrorHandler
    popa
    ret

; ================================================================= ;
;                         boot sector data
; ================================================================= ;
DiskReadFailedMessage:
    db 'failed to read disk', CRLF, 0

BootloaderNotFoundMessage:
    db 'bootld.bin not found', CRLF, 0

DiskLoadingMessage:
    db 'Bootsector..', CRLF, 0

BootloaderFileName:
    db 'bootld  bin'

; ---------------------------------------
; pad with 0 until you reach address 0x7DFE
; ---------------------------------------
times 510 - ($ - $$) db 0

; -----------------
; 0x7DFE, boot sector signature, 2 bytes
; -----------------
dw 0xAA55

; -----------------------------------------------
; 0x7E00, from here on, nothing is loaded into the floppy disk
; -----------------------------------------------
BootloaderLogicalCluster:
    dw 0
RootDirectoryBuffer:
FAT1Buffer: