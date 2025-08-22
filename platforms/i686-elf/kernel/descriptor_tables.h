#pragma once

extern gdt_entry GlobalRamGDT[3];
extern gdt_descriptor GlobalGDTDescriptor;

extern idt_entry GlobalRamIDT[256];
extern idt_descriptor GlobalIDTDescriptor;

void InitializeRamGDT();
void InitializeRamIDT();
void EnableAllISRs();