#pragma once

b8 __attribute__((cdecl)) BIOSDiskReset(u8 DriveNumber);

b8 __attribute__((cdecl)) BIOSDiskRead
(
    u8 DriveNumber, u16 Cylinder, u16 Head,
    u16 Sector, u8 SectorCount, u8 *DataOut
);

b8 __attribute__((cdecl)) BIOSDiskWrite
(
    u8 DriveNumber, u16 Cylinder, u16 Head,
    u16 Sector, u8 SectorCount, u8 *DataIn
);

b8 __attribute__((cdecl)) BIOSGetDiskDriveParameters
(
    u8 DriveNumber, u8 *DriveTypeOut, u16 *CylindersOut,
    u16 *SectorsOut, u16 *HeadsOut
);