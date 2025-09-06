#include "i686-elf\shared\base_types.h"
#include "i686-elf\shared\basic_defines.h"
#include "i686-elf\shared\bios\disk.h"
#include "i686-elf\shared\storage\disk\disk.h"

void GetDiskDriveParameters(disk_parameters *DiskParameters, u8 DriveNumber)
{
    b8 Result = FALSE;
    u8 DriveType;
    u16 Cylinders;
    u16 Sectors;
    u16 Heads;

    DiskParameters->Id = DriveNumber;
    Result = BIOSGetDiskDriveParameters(DriveNumber, &DriveType, &Cylinders, &Sectors, &Heads);
    if (!Result)
    {
        return;
    }

    DiskParameters->Cylinders = Cylinders;
    DiskParameters->Heads = Heads + 1;
    DiskParameters->Sectors = Sectors;
    DiskParameters->Type = DriveType;
}

void TranslateLbaToChs
(
    disk_parameters *DiskParameters,
    u32 LogicalBlockAddress, u16 *Cylinder, u16 *Head, u16 *Sector
)
{
    *Cylinder = (LogicalBlockAddress / DiskParameters->Sectors) / DiskParameters->Heads;
    *Head = (LogicalBlockAddress / DiskParameters->Sectors) % DiskParameters->Heads;
    *Sector = (LogicalBlockAddress % DiskParameters->Sectors) + 1; // sector indices start from 1
}

void ReadDiskSectors
(
    disk_parameters *DiskParameters,
    u32 LogicalBlockAddress,
    u8 SectorsToRead,
    void *DataOut
)
{
    u16 Cylinder, Head, Sector;
    TranslateLbaToChs(DiskParameters, LogicalBlockAddress, &Cylinder, &Head, &Sector);

    for (u16 Retry = 0; Retry < DISK_OPERATION_MAXIMUM_RETRIES; Retry++)
    {
        b8 ReadOk = BIOSDiskRead
        (
            DiskParameters->Id,
            Cylinder, Head, Sector,
            SectorsToRead, DataOut
        );

        if (ReadOk)
        {
            return;
        }
        else
        {
            BIOSDiskReset(DiskParameters->Id);
        }
    }
}

void WriteDiskSectors
(
    disk_parameters *DiskParameters,
    u32 LogicalBlockAddress,
    u8 SectorsToWrite,
    u8 *DataIn
)
{
    u16 Cylinder, Head, Sector;
    TranslateLbaToChs(DiskParameters, LogicalBlockAddress, &Cylinder, &Head, &Sector);

    for (u16 Retry = 0; Retry < DISK_OPERATION_MAXIMUM_RETRIES; Retry++)
    {
        b8 WriteOk = BIOSDiskWrite
        (
            DiskParameters->Id,
            Cylinder, Head, Sector,
            SectorsToWrite, DataIn
        );

        if (WriteOk)
        {
            return;
        }
        else
        {
            BIOSDiskReset(DiskParameters->Id);
        }
    }
}