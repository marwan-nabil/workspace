extern ISREntry

%macro ISRWithoutErrorCode 1
global ISR%1
ISR%1:
    push 0 ; dummy error code
    push %1
    jmp ISRsCommon
%endmacro

%macro ISRWithErrorCode 1
global ISR%1
ISR%1:
    push %1
    jmp ISRsCommon
%endmacro