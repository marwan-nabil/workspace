"this file defines the nasm_toolchain rule"

def _nasm_toolchain_impl(ctx):
    return [platform_common.ToolchainInfo(
        nasm = ctx.executable.nasm,
    )]

nasm_toolchain = rule(
    implementation = _nasm_toolchain_impl,
    attrs = {
        "nasm": attr.label(executable = True, cfg = "exec"),
    },
)
