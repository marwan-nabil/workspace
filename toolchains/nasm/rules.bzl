"this file defines the nasm_assemble bazel rule"

def _nasm_assemble_impl(ctx):
    nasm_exe = ctx.toolchains[":nasm_toolchain_type"].nasm
    output = ctx.actions.declare_file(ctx.label.name + ".img")
    ctx.actions.run(
        inputs = ctx.files.srcs,
        outputs = [output],
        executable = nasm_exe,
        arguments = ["-f", "bin", ctx.files.srcs[0].path, "-o", output.path],
    )
    return [DefaultInfo(files = depset([output]))]

nasm_assemble = rule(
    implementation = _nasm_assemble_impl,
    attrs = {"srcs": attr.label_list(allow_files = [".s"])},
    toolchains = [":nasm_toolchain_type"],
)
