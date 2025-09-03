#pragma once

#define DISK_OPERATION_MAXIMUM_RETRIES 3

typedef struct
{
    u8 Id;
    u8 Type;
    u16 Cylinders;
    u16 Heads;
    u16 Sectors;
} disk_parameters;

void GetDiskDriveParameters(disk_parameters *DiskParameters, u8 DriveNumber);
void TranslateLbaToChs
(
    disk_parameters *DiskParameters,
    u32 LogicalBlockAddress, u16 *Cylinder, u16 *Head, u16 *Sector
);

void GetDiskDriveParameters(disk_parameters *DiskParameters, u8 DriveNumber);
void TranslateLbaToChs
(
    disk_parameters *DiskParameters,
    u32 LogicalBlockAddress, u16 *Cylinder, u16 *Head, u16 *Sector
);
void ReadDiskSectors
(
    disk_parameters *DiskParameters,
    u32 LogicalBlockAddress,
    u8 SectorsToRead,
    void *DataOut
);
void WriteDiskSectors
(
    disk_parameters *DiskParameters,
    u32 LogicalBlockAddress,
    u8 SectorsToWrite,
    u8 *DataIn
);