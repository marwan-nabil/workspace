#pragma once

void __attribute__((cdecl)) WriteByteToOutputPort(u16 Port, u8 Value);
u8 __attribute__((cdecl)) ReadByteFromOutputPort(u16 Port);