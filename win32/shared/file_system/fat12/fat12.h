#pragma once

#include "portable\shared\base_types.h"

#define FAT12_SECTORS_PER_CLUSTER 1
#define FAT12_SECTORS_PER_FAT 9
#define FAT12_SECTORS_IN_ROOT_DIRECTORY 14
#define FAT12_SECTORS_IN_DATA_AREA 2847
#define FAT12_TOTAL_DISK_SECTORS 2880

#define FAT12_DIRECTORY_ENTRIES_PER_SECTOR 16
#define FAT12_DATA_AREA_START_SECTOR 33

#define FAT12_SECTOR_SIZE 512
#define FAT12_CLUSTER_SIZE (FAT12_SECTOR_SIZE * FAT12_SECTORS_PER_CLUSTER)
#define FAT12_FAT_SIZE (FAT12_SECTOR_SIZE * FAT12_SECTORS_PER_FAT)

#define FAT12_BOOT_SIGNATURE 0xAA55
#define FAT12_ENTRIES_PER_FAT 3072

#define FAT12_FILENAME_EMPTY_SLOT 0x00
#define FAT12_FILENAME_DELETED_SLOT 0xE5

#define FAT12_FILE_ATTRIBUTE_NORMAL 0x00
#define FAT12_FILE_ATTRIBUTE_READONLY 0x01
#define FAT12_FILE_ATTRIBUTE_HIDDEN 0x02
#define FAT12_FILE_ATTRIBUTE_SYSTEM 0x04
#define FAT12_FILE_ATTRIBUTE_VOLUME 0x08
#define FAT12_FILE_ATTRIBUTE_DIRECTORY 0x10
#define FAT12_FILE_ATTRIBUTE_ARCHIVE 0x20

#define FAT12_FAT_ENTRY_FREE_CLUSTER 0x0000
#define FAT12_FAT_ENTRY_RESERVED_CLUSTER 0x0FF6
#define FAT12_FAT_ENTRY_BAD_CLUSTER 0x0FF7
#define FAT12_FAT_ENTRY_END_OF_FILE_CLUSTER_RANGE_START 0x0FF8
#define FAT12_FAT_ENTRY_END_OF_FILE_CLUSTER_RANGE_END 0x0FFF

#pragma pack(push, 1)

struct boot_sector
{
    u8 JumpInstructionSpace[3];
    u8 OEMName[8];

    u16 BytesPerSector;
    u8 SectorsPerCluster;
    u16 NumberOfReserevedSectors;
    u8 NumberOfFATs;
    u16 RootDirectoryEntries;
    u16 TotalSectors;
    u8 MediaDescriptor;
    u16 SectorsPerFAT;
    u16 SectorsPerTrack;
    u16 NumberOfHeads;
    u32 HiddenSectors;
    u32 LargeSectors;

    u8 DriveNumber;
    u8 Reserved;
    u8 Signature;
    u32 VolumeId;
    u8 VolumeLabel[11];
    u8 SystemId[8];

    u8 BootSectorCode[448];
    u16 BootSectorSignature;
};

struct directory_entry
{
    u8 FileName[8];
    u8 FileExtension[3];
    u8 FileAttributes;
    u8 Reserved[2];
    u16 CreationTime;
    u16 CreationDate;
    u16 LastAccessDate;
    u16 ClusterNumberHighWord;
    u16 LastWriteTime;
    u16 LastWriteDate;
    u16 ClusterNumberLowWord;
    u32 FileSize;
};

union sector
{
    u8 Bytes[FAT12_SECTOR_SIZE];
    directory_entry DirectoryEntries[FAT12_DIRECTORY_ENTRIES_PER_SECTOR];
};

union file_allocation_table
{
    sector Sectors[FAT12_SECTORS_PER_FAT];
    u8 Bytes[FAT12_FAT_SIZE];
};

struct root_directory
{
    sector Sectors[FAT12_SECTORS_IN_ROOT_DIRECTORY];
};

struct data_area
{
    sector Sectors[FAT12_SECTORS_IN_DATA_AREA];
};

union fat12_disk
{
    struct
    {
        boot_sector BootSector;
        file_allocation_table Fat1;
        file_allocation_table Fat2;
        root_directory RootDirectory;
        data_area DataArea;
    };
    sector Sectors[FAT12_TOTAL_DISK_SECTORS];
};

#pragma pack(pop)