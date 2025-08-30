"this file defines the nasm_assemble bazel rule"

def _nasm_assemble_impl(ctx):
    nasm_exe = ctx.toolchains["@nasm//:nasm_toolchain_type"].assembler
    image_file = ctx.actions.declare_file(ctx.label.name + ".img")
    listing_file = ctx.actions.declare_file(ctx.attr.listing_file)
    ctx.actions.run(
        inputs = ctx.files.srcs,
        outputs = [image_file, listing_file],
        executable = nasm_exe,
        arguments = ["-f", "bin", ctx.files.srcs[0].path, "-o", image_file.path, "-l", listing_file.path],
    )
    return [DefaultInfo(files = depset([image_file, listing_file]))]

nasm_assemble = rule(
    implementation = _nasm_assemble_impl,
    attrs = {
        "srcs": attr.label_list(allow_files = [".s"]),
        "listing_file": attr.string(),
    },
    toolchains = ["@nasm//:nasm_toolchain_type"],
)

def _nasm_disassemble_impl(ctx):
    ndisasm_exe = ctx.toolchains["@nasm//:nasm_toolchain_type"].disassembler
    assembler_outputs = ctx.attr.src[DefaultInfo].files.to_list()
    image_files = [file for file in assembler_outputs if file.extension == "img"]
    disassembly_files = []

    for image_file in image_files:
        disassembly_file = ctx.actions.declare_file(image_file.basename + ".disasm")
        disassembly_files.append(disassembly_file)
        ctx.actions.run_shell(
            inputs = [image_file],
            outputs = [disassembly_file],
            arguments = [image_file.path, disassembly_file.path],
            command = ndisasm_exe.path + " $1 > $2",
        )

    return [DefaultInfo(files = depset(disassembly_files))]

nasm_disassemble = rule(
    implementation = _nasm_disassemble_impl,
    attrs = {
        "src": attr.label(),
    },
    toolchains = ["@nasm//:nasm_toolchain_type"],
)
