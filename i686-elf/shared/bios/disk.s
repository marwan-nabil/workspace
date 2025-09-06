%include "i686-elf\shared\cpu\modes.i"

; ---------------------------------------
; resets the floppy disk
; b8 __attribute__((cdecl)) BIOSDiskReset(u8 DriveNumber);
; ---------------------------------------
section .text
global BIOSDiskReset
BIOSDiskReset:
    [bits 32]
    push ebp
    mov ebp, esp

    x86EnterRealMode

    mov ah, 0
    mov dl, [bp + 8] ; drive number
    stc
    int 0x13

    mov eax, 1
    ; carry flag is 1 in case of error in the BIOS function
    sbb eax, 0 ; subtract the carry flag from ax

    push eax
    x86EnterProtectedMode
    pop eax

    mov esp, ebp
    pop ebp
    ret

; ---------------------------------------
; reads sectors from floppy disk given the
; CHS and the sector count
; void __attribute__((cdecl)) BIOSDiskRead
; (
;     u8 DriveNumber, u16 Cylinder, u16 Head,
;     u16 Sector, u8 SectorCount, u8 *DataOut
; );
; ---------------------------------------
section .text
global BIOSDiskRead
BIOSDiskRead:
    [bits 32]
    push ebp
    mov ebp, esp

    x86EnterRealMode

    ; save touched registers
    push ebx
    push es

    ; setup arguments for the BIOS
    ; read disk service
    mov dl, [bp + 8] ; drive number

    mov ch, [bp + 12] ; cylinder[7:0]
    mov cl, [bp + 13] ; cylinder[15:8]
    shl cl, 6

    mov dh, [bp + 16] ; head

    mov al, [bp + 20] ; sector
    and al, 0x3F
    or cl, al ; cl == {cylinder[9:8], sector[5:0]}

    mov al, [bp + 24] ; SectorCount

    ConvertLinearAddressToSegmentOffsetAddress [bp + 28], es, ebx, bx

    mov ah, 0x02 ; disk service for reading
    stc
    int 0x13

    mov eax, 1
    ; carry flag is 1 in case of error in the BIOS function
    sbb eax, 0 ; subtract the carry flag from ax

    ; restore touched registers
    pop es
    pop ebx

    push eax
    x86EnterProtectedMode
    pop eax

    mov esp, ebp
    pop ebp
    ret

; ---------------------------------------
; writes sectors to floppy disk given the
; CHS and the sector count
; void __attribute__((cdecl)) BIOSDiskWrite
; (
;     u8 DriveNumber, u16 Cylinder, u16 Head,
;     u16 Sector, u8 SectorCount, u8 *DataIn
; );
; ---------------------------------------
section .text
global BIOSDiskWrite
BIOSDiskWrite:
    [bits 32]
    push ebp
    mov ebp, esp

    x86EnterRealMode

    ; save touched registers
    push ebx
    push es

    ; setup arguments for the BIOS
    ; read disk service
    mov dl, [bp + 8] ; drive number

    mov ch, [bp + 12] ; cylinder[7:0]
    mov cl, [bp + 13] ; cylinder[15:8]
    shl cl, 6

    mov dh, [bp + 16] ; head

    mov al, [bp + 20] ; sector
    and al, 0x3F
    or cl, al ; cl == {cylinder[9:8], sector[5:0]}

    mov al, [bp + 24] ; SectorCount

    ConvertLinearAddressToSegmentOffsetAddress [bp + 28], es, ebx, bx

    mov ah, 0x03 ; disk service for writing
    stc
    int 0x13

    mov eax, 1
    ; carry flag is 1 in case of error in the BIOS function
    sbb eax, 0 ; subtract the carry flag from ax

    ; restore touched registers
    pop es
    pop ebx

    push eax
    x86EnterProtectedMode
    pop eax

    mov esp, ebp
    pop ebp
    ret

; ---------------------------------------
; void __attribute__((cdecl)) BIOSGetDiskDriveParameters
; (
;     u8 DriveNumber, u8 *DriveTypeOut, u16 *CylindersOut,
;     u16 *SectorsOut, u16 *HeadsOut
; );
; ---------------------------------------
section .text
global BIOSGetDiskDriveParameters
BIOSGetDiskDriveParameters:
    [bits 32]
    push ebp
    mov ebp, esp

    x86EnterRealMode

    ; save registers
    push es
    push bx
    push esi
    push di

    ; prepare arguments for BIOS service that
    ; gets the drive parameters
    mov dl, [bp + 8] ; drive number
    mov ah, 0x08 ; BIOS service number
    ; es:di == 0 to avoid BIOS bugs
    mov di, 0
    mov es, di
    stc
    int 0x13

    mov eax, 1
    sbb eax, 0

    ConvertLinearAddressToSegmentOffsetAddress [bp + 12], es, esi, si
    ; store results of BIOS service
    mov [es:si], bl ; drive type

    mov bl, ch ; maximum cylinder number [7:0]
    mov bh, cl ; maximum cylinder number [9:8]
    shr bh, 6
    inc bx

    ConvertLinearAddressToSegmentOffsetAddress [bp + 16], es, esi, si
    mov [es:si], bx ; bx[9:0] == maximum number of cylinders

    xor ch, ch
    and cl, 0x3F

    ConvertLinearAddressToSegmentOffsetAddress [bp + 20], es, esi, si
    mov [es:si], cx ; cx[5:0] == maximum sector number

    mov cl, dh
    inc cx

    ConvertLinearAddressToSegmentOffsetAddress [bp + 24], es, esi, si
    mov [es:si], cx ; cx == maximum number of heads

    ; restore registers
    pop di
    pop esi
    pop bx
    pop es

    push eax
    x86EnterProtectedMode
    pop eax

    mov esp, ebp
    pop ebp
    ret