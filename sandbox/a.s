; .global _main
.global _start
.align 4

.text
_start:
    STP     x29, x30, [sp, -48]!

    MOV     x29, sp
    MOV     x6, 0 // flag
    STR     x6, [sp, 16]

    gotoSCheck:
        ldr     x6, [sp, 16]
        cmp     x6, 0
        b.eq    branch1 // branch only if flag is 0

    MOV     x1, a
    MOV     x2, b

    MUL     x1, x1, x2
    STR     x1, [sp]

    ADRP    x0,    fmt@PAGE
    ADD     x0, x0, fmt@PAGEOFF

    BL      _printf

branch1:
    // if  x6 == 1
    LDR     x6, [sp, 16]
    CMP     x6, 1
    B.EQ    b1Print

    //else
    MOV     x6, 1 // set flag
    STR     x6, [sp, 16]
    MOV     x7, 8

    // If x7 != 0 (always true)
    CMP     x7, 0
    B.NE    gotoSCheck

    b1Print:
        ADRP    x0, bout_str@PAGE
        ADD     x0, x0, bout_str@PAGEOFF
        BL      _printf 

.data
fmt_print_loop: .string "Printing %d numbers: \n"
fmt_num_prefix: .string "Num %d \n"
// decrement 1 from dec_num while > 0

.text
print_loop:
    STP     x29, x30, [sp, -64]!
    MOV     x29, sp


    ADRP    x0, fmt_print_loop@PAGE
    ADD     x0, x0, fmt_print_loop@PAGEOFF
    MOV     x20, dec_num
    STR     x20, [x29]
    BL      _printf
    
//  
// do
    loop:

    SUB     x20, x20, 1 // i--
    SUB     sp, sp, 16
    STR     x20, [sp]
    ADRP    x0, fmt_num_prefix@PAGE
    ADD     x0, x0, fmt_num_prefix@PAGEOFF
    BL      _printf
    ADD     sp, sp, 16


// while
    ; LDR     x20, [x29]
    CMP     x20, 0
    B.GT    loop

    LDP     x29, x30, [sp], 64

.data
hw: .string "Hello World!"
hw_len = . -hw - 1
strlen_fmt: .string "The string \"%s\" is %d characters long!\n"

.text
_end:
    STP     x29, x30, [sp, -48]!
    MOV     x29, sp


    ADRP    x0, strlen_fmt@PAGE
    ADD     x0, x0, strlen_fmt@PAGEOFF

    ADRP    x1, hw@PAGE
    ADD     x1, x1, hw@PAGEOFF
    STR     x1, [x29]

    MOV     x2, hw_len
    STR     x2, [x29, 8]

    BL      _printf
    LDP     x29, x30, [sp], 48


    LDP     x29, x30, [sp], 80
    MOV     x0, 69
    BL      _exit


.data
fmt: .string "%d\n" // make type dynamic with parser
bout_str: .string "Second condition!\n"


a = 4
b = 25
dec_num = 100
