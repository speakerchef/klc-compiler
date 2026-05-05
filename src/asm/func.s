.global _main
.align 4

.text ; funcs use x8-x15 for arguments
my_func: ; args: a - exit code; exit (val * 5) + (val + 10)
    stp     x29, x30, [sp, -64]!
    mov     x29, sp
    stp     x19, x20, [sp, 48]
    mov     x19, 5
    mul     x19, x19, x8
    mov     x20, 10
    add     x20, x20, x8
    add     x0, x19, x20
    bl      _exit
    ldp     x19, x20, [sp, 48]
    ldp     x29, x30, [sp], 64
    ret


_main:
    mov     x8, 1
    bl      my_func
