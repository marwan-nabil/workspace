"this file defines the nasm_toolchain rule"

def _i686_dash_elf_toolchain_rule_impl(ctx):
    return [platform_common.ToolchainInfo()]

i686_dash_elf_toolchain_rule = rule(
    implementation = _i686_dash_elf_toolchain_rule_impl,
    attrs = {
        "nasm_exe": attr.label(executable = True, cfg = "exec"),
        "ndisasm_exe": attr.label(executable = True, cfg = "exec"),
    },
)
