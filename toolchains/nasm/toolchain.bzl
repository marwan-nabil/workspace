"this file defines the nasm_toolchain rule"

def _nasm_toolchain_rule_impl(ctx):
    return [platform_common.ToolchainInfo(
        assembler = ctx.executable.nasm_exe,
        disassembler = ctx.executable.ndisasm_exe,
    )]

nasm_toolchain_rule = rule(
    implementation = _nasm_toolchain_rule_impl,
    attrs = {
        "nasm_exe": attr.label(executable = True, cfg = "exec"),
        "ndisasm_exe": attr.label(executable = True, cfg = "exec"),
    },
)
