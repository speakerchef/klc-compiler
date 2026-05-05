.global _main
.align 4
_main:
	SUB sp, sp, 96
	STR x29, [sp, 0]
	STR x30, [sp, 8]
	MOV x29, sp
	MOV x28, x29
	MOV x8, 0x05
	STR x8, [x29, 80]
	MOV x8, 0x01
	STR x8, [x29, 72]
	LDR x8, [x29, 80]
	LDR x9, [x29, 72]
	CMP x8, x9
	CSET x8, EQ
	STR x8, [x29, 64]
	LDR x8, [x29, 64]
	CMP x8, 0
	B.NE label_if0
	B label_elif1

label_if0:
	MOV x8, 0x0a
	STR x8, [x29, 56]
	LDR x0, [x29, 56]
	MOV x16, 1
	LDR x29, [sp, 0]
	LDR x30, [sp, 8]
	ADD sp, sp, 96
	BL  _exit
	B label_end0
label_elif1:
	MOV x8, 0x02
	STR x8, [x29, 48]
	LDR x8, [x29, 80]
	LDR x9, [x29, 48]
	CMP x8, x9
	CSET x8, EQ
	STR x8, [x29, 40]
	LDR x8, [x29, 40]
	CMP x8, 0
	B.EQ label_elif2

	MOV x8, 0x14
	STR x8, [x29, 32]
	LDR x0, [x29, 32]
	MOV x16, 1
	LDR x29, [sp, 0]
	LDR x30, [sp, 8]
	ADD sp, sp, 96
	BL  _exit
	B label_end0
label_elif2:
	MOV x8, 0x03
	STR x8, [x29, 24]
	LDR x8, [x29, 80]
	LDR x9, [x29, 24]
	CMP x8, x9
	CSET x8, EQ
	STR x8, [x29, 16]
	LDR x8, [x29, 16]
	CMP x8, 0
	B.EQ label_elif3

	MOV x8, 0x1e
	STR x8, [x29, 8]
	LDR x0, [x29, 8]
	MOV x16, 1
	LDR x29, [sp, 0]
	LDR x30, [sp, 8]
	ADD sp, sp, 96
	BL  _exit
	B label_end0
label_elif3:
	MOV x8, 0x04
	STR x8, [x29, 0]
	LDR x8, [x29, 80]
	LDR x9, [x29, 0]
	CMP x8, x9
	CSET x8, EQ
	STR x8, [x29, -8]
	LDR x8, [x29, -8]
	CMP x8, 0
	B.EQ label_elif4

	MOV x8, 0x28
	STR x8, [x29, -16]
	LDR x0, [x29, -16]
	MOV x16, 1
	LDR x29, [sp, 0]
	LDR x30, [sp, 8]
	ADD sp, sp, 96
	BL  _exit
	B label_end0
label_elif4:
	MOV x8, 0x05
	STR x8, [x29, -24]
	LDR x8, [x29, 80]
	LDR x9, [x29, -24]
	CMP x8, x9
	CSET x8, EQ
	STR x8, [x29, -32]
	LDR x8, [x29, -32]
	CMP x8, 0
	B.EQ label_else0
	MOV x8, 0x32
	STR x8, [x29, -40]
	LDR x0, [x29, -40]
	MOV x16, 1
	LDR x29, [sp, 0]
	LDR x30, [sp, 8]
	ADD sp, sp, 96
	BL  _exit
	B label_end0
label_else0:
	MOV x8, 0x63
	STR x8, [x29, -48]
	LDR x0, [x29, -48]
	MOV x16, 1
	LDR x29, [sp, 0]
	LDR x30, [sp, 8]
	ADD sp, sp, 96
	BL  _exit
label_end0:
