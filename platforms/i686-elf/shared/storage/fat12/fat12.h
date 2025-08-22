#pragma once

#define FAT12_SECTOR_SIZE 512
#define FAT12_DIRECTORY_ENTRIES_IN_SECTOR 16
#define FAT12_FAT_ENTRIES_IN_FAT 3072

#define FAT12_SECTORS_IN_FAT 9
#define FAT12_SECTORS_IN_ROOT_DIRECTORY 14
#define FAT12_SECTORS_IN_DATA_AREA 2847
#define FAT12_SECTORS_IN_DISK 2880

#define FAT12_FAT_SIZE (FAT12_SECTOR_SIZE * FAT12_SECTORS_IN_FAT)
#define FAT12_ROOT_DIRECTORY_SIZE (FAT12_SECTOR_SIZE * FAT12_SECTORS_IN_ROOT_DIRECTORY)
#define FAT12_DATA_AREA_SIZE (FAT12_SECTOR_SIZE * FAT12_SECTORS_IN_DATA_AREA)

#define FAT12_DATA_AREA_START_SECTOR 33
#define FAT12_BOOT_SIGNATURE 0xAA55

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

typedef struct __attribute__((packed))
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
} boot_sector_header;

typedef struct __attribute__((packed))
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
} boot_sector;

typedef struct __attribute__((packed))
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
} directory_entry;

typedef union __attribute__((packed))
{
    u8 Bytes[FAT12_SECTOR_SIZE];
    directory_entry DirectoryEntries[FAT12_DIRECTORY_ENTRIES_IN_SECTOR];
} sector;

typedef union __attribute__((packed))
{
    sector Sectors[FAT12_SECTORS_IN_FAT];
    u8 Bytes[FAT12_FAT_SIZE];
} file_allocation_table;

typedef struct __attribute__((packed))
{
    sector Sectors[FAT12_SECTORS_IN_ROOT_DIRECTORY];
} root_directory;

typedef struct
{
    boot_sector_header BootSectorHeader;
    file_allocation_table Fat;
    root_directory RootDirectory;
} fat12_ram_disk;