#include <stdarg.h>
#include "i686-elf\shared\base_types.h"
#include "i686-elf\shared\basic_defines.h"
#include "i686-elf\shared\strings\print.h"
#include "i686-elf\shared\cpu\panic.h"
#include "i686-elf\os\kernel\isr.h"
#include "i686-elf\os\kernel\main.h"

isr_handler_function GlobalISRHandlers[256];
char *GlobalExceptionMessages[] =
{
    "Divide by zero exeption",
    "Debug",
    "Non-maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "",
    "x87 Floating-Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Control Protection Exception ",
    "",
    "",
    "",
    "",
    "",
    "",
    "Hypervisor Injection Exception",
    "VMM Communication Exception",
    "Security Exception",
    ""
};

void __attribute__((cdecl)) ISREntry(isr_passed_context *Context)
{
    if (GlobalISRHandlers[Context->InterruptNumber])
    {
        GlobalISRHandlers[Context->InterruptNumber](Context);
    }
    else if (Context->InterruptNumber < 32)
    {
        PrintFormatted
        (
            &GlobalPrintContext,
            "UNHANDLED EXCEPTION: %d, %s.\n",
            Context->InterruptNumber,
            GlobalExceptionMessages[Context->InterruptNumber]
        );
        PrintFormatted
        (
            &GlobalPrintContext,
            "CONTEXT DUMP:\n"
            "eax = 0x%x \t\t ebx = 0x%x \t\t ecx = 0x%x\n"
            "edx = 0x%x \t\t esi = 0x%x \t\t edi = 0x%x\n"
            "esp = 0x%x \t\t ebp = 0x%x \t\t eip = 0x%x\n",
            Context->Register_eax, Context->Register_ebx,
            Context->Register_ecx, Context->Register_edx,
            Context->Register_esi, Context->Register_edi,
            Context->Register_esp, Context->Register_ebp,
            Context->Register_eip
        );
        PrintFormatted
        (
            &GlobalPrintContext,
            "ss = 0x%x \t cs = 0x%x \t ds = 0x%x\n"
            "eflags = 0x%x\n",
            Context->Register_ss, Context->Register_cs, Context->Register_ds,
            Context->Register_eflags
        );
        PrintFormatted
        (
            &GlobalPrintContext,
            "   InterruptNumber = 0x%x  ErrorCode = 0x%x\n",
            Context->InterruptNumber, Context->ErrorCode
        );
        Panic();
    }
    else
    {
        PrintFormatted
        (
            &GlobalPrintContext,
            "ISR: %d, has no handler, time to panic.\n",
            Context->InterruptNumber
        );
        Panic();
    }
}