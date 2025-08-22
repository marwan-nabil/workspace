#pragma once

void TestVGA(print_context *PrintContext);
void TestIO(print_context *PrintContext);
void StringTests(print_context *PrintContext);
void DiskDriverTests(u8 BootDriveNumber, void *FreeMemoryArea, print_context *PrintContext);
void AllocatorTests(void *FreeMemoryAddress, print_context *PrintContext);
void PathHandlingTests(void *FreeMemoryArea, print_context *PrintContext);
void Fat12Tests(u16 BootDriveNumber, void *FreeMemoryArea, print_context *PrintContext);
void FileIoTests(u16 BootDriveNumber, void *FreeMemoryArea, print_context *PrintContext);