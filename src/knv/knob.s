.global _main
.align 4
_main:
	SUB sp, sp, 1696
	MOV x8, 1
	STR x8, [sp, 1680]
	MOV x8, 0
	STR x8, [sp, 1672]
label2while:
	MOV x8, 50000
	STR x8, [sp, 1664]
	LDR x8, [sp, 1672]
	LDR x9, [sp, 1664]
	CMP x8, x9
	CSET x8, LT
	STR x8, [sp, 1656]
	LDR x8, [sp, 1656]
	CMP x8, 0
	B.NE label3
	B label_end4
label3:
	LDR x10, [sp, 1672]
	STR x10, [sp, 1648]
	MOV x8, 0
	STR x8, [sp, 1640]
label7while:
	MOV x8, 0
	STR x8, [sp, 1632]
	LDR x8, [sp, 1648]
	LDR x9, [sp, 1632]
	CMP x8, x9
	CSET x8, GT
	STR x8, [sp, 1624]
	LDR x8, [sp, 1624]
	CMP x8, 0
	B.NE label8
	B label_end9
label8:
	MOV x8, 1
	STR x8, [sp, 1616]
	LDR x8, [sp, 1648]
	LDR x9, [sp, 1616]
	AND x8, x8, x9
	STR x8, [sp, 1608]
	LDR x8, [sp, 1608]
	CMP x8, 0
	B.NE label_if10
	B label_else11
label_if10:
	MOV x8, 1
	STR x8, [sp, 1600]
	LDR x8, [sp, 1640]
	LDR x9, [sp, 1600]
	ADD x8, x8, x9
	STR x8, [sp, 1640]
	B label_end12
label_else11:
	B label_end12
label_end12:
	MOV x8, 2
	STR x8, [sp, 1592]
	LDR x8, [sp, 1648]
	LDR x9, [sp, 1592]
	SDIV x8, x8, x9
	STR x8, [sp, 1648]
	B label7while
label_end9:
	MOV x8, 8
	STR x8, [sp, 1584]
	LDR x8, [sp, 1640]
	LDR x9, [sp, 1584]
	CMP x8, x9
	CSET x8, GT
	STR x8, [sp, 1576]
	LDR x8, [sp, 1576]
	CMP x8, 0
	B.NE label_if13
	B label_elif16
label_if13:
	MOV x8, 31
	STR x8, [sp, 1568]
	LDR x8, [sp, 1680]
	LDR x9, [sp, 1568]
	MUL x8, x8, x9
	STR x8, [sp, 1560]
	LDR x8, [sp, 1560]
	LDR x9, [sp, 1672]
	ADD x8, x8, x9
	STR x8, [sp, 1552]
	MOVZ x8, 0xffff
	MOVK x8, 0x00, LSL 16
	STR x8, [sp, 1544]
	LDR x8, [sp, 1552]
	LDR x9, [sp, 1544]
	AND x8, x8, x9
	STR x8, [sp, 1680]
	B label_end15
label_elif16:
	MOV x8, 4
	STR x8, [sp, 1536]
	LDR x8, [sp, 1640]
	LDR x9, [sp, 1536]
	CMP x8, x9
	CSET x8, GT
	STR x8, [sp, 1528]
	LDR x8, [sp, 1528]
	CMP x8, 0
	B.NE label_branch17
	B label_else14
label_branch17:
	LDR x8, [sp, 1680]
	LDR x9, [sp, 1672]
	EOR x8, x8, x9
	STR x8, [sp, 1520]
	MOVZ x8, 0xffff
	MOVK x8, 0x00, LSL 16
	STR x8, [sp, 1512]
	LDR x8, [sp, 1520]
	LDR x9, [sp, 1512]
	AND x8, x8, x9
	STR x8, [sp, 1680]
	B label_end15
label_else14:
	MOV x8, 7
	STR x8, [sp, 1504]
	LDR x8, [sp, 1640]
	LDR x9, [sp, 1504]
	MUL x8, x8, x9
	STR x8, [sp, 1496]
	LDR x8, [sp, 1680]
	LDR x9, [sp, 1496]
	ADD x8, x8, x9
	STR x8, [sp, 1488]
	MOVZ x8, 0xffff
	MOVK x8, 0x00, LSL 16
	STR x8, [sp, 1480]
	LDR x8, [sp, 1488]
	LDR x9, [sp, 1480]
	AND x8, x8, x9
	STR x8, [sp, 1680]
	B label_end15
label_end15:
	MOV x8, 1
	STR x8, [sp, 1472]
	LDR x8, [sp, 1672]
	LDR x9, [sp, 1472]
	ADD x8, x8, x9
	STR x8, [sp, 1464]
	MOV x8, 0
	STR x8, [sp, 1456]
label20while:
	MOV x8, 1
	STR x8, [sp, 1448]
	LDR x8, [sp, 1464]
	LDR x9, [sp, 1448]
	CMP x8, x9
	CSET x8, NE
	STR x8, [sp, 1440]
	LDR x8, [sp, 1440]
	CMP x8, 0
	B.NE label21
	B label_end22
label21:
	MOV x8, 1
	STR x8, [sp, 1432]
	LDR x8, [sp, 1464]
	LDR x9, [sp, 1432]
	AND x8, x8, x9
	STR x8, [sp, 1424]
	MOV x8, 1
	STR x8, [sp, 1416]
	LDR x8, [sp, 1424]
	LDR x9, [sp, 1416]
	CMP x8, x9
	CSET x8, EQ
	STR x8, [sp, 1408]
	LDR x8, [sp, 1408]
	CMP x8, 0
	B.NE label_if23
	B label_else24
label_if23:
	MOV x8, 3
	STR x8, [sp, 1400]
	LDR x8, [sp, 1464]
	LDR x9, [sp, 1400]
	MUL x8, x8, x9
	STR x8, [sp, 1392]
	MOV x8, 1
	STR x8, [sp, 1384]
	LDR x8, [sp, 1392]
	LDR x9, [sp, 1384]
	ADD x8, x8, x9
	STR x8, [sp, 1464]
	B label_end25
label_else24:
	MOV x8, 2
	STR x8, [sp, 1376]
	LDR x8, [sp, 1464]
	LDR x9, [sp, 1376]
	SDIV x8, x8, x9
	STR x8, [sp, 1464]
	B label_end25
label_end25:
	MOV x8, 1
	STR x8, [sp, 1368]
	LDR x8, [sp, 1456]
	LDR x9, [sp, 1368]
	ADD x8, x8, x9
	STR x8, [sp, 1456]
	B label20while
label_end22:
	LDR x8, [sp, 1680]
	LDR x9, [sp, 1456]
	ADD x8, x8, x9
	STR x8, [sp, 1360]
	MOVZ x8, 0xffff
	MOVK x8, 0x00, LSL 16
	STR x8, [sp, 1352]
	LDR x8, [sp, 1360]
	LDR x9, [sp, 1352]
	AND x8, x8, x9
	STR x8, [sp, 1680]
	MOV x8, 17
	STR x8, [sp, 1344]
	LDR x8, [sp, 1672]
	LDR x9, [sp, 1344]
	ADD x8, x8, x9
	STR x8, [sp, 1336]
	MOV x8, 31
	STR x8, [sp, 1328]
	LDR x8, [sp, 1672]
	LDR x9, [sp, 1328]
	ADD x8, x8, x9
	STR x8, [sp, 1320]
label28while:
	MOV x8, 0
	STR x8, [sp, 1312]
	LDR x8, [sp, 1320]
	LDR x9, [sp, 1312]
	CMP x8, x9
	CSET x8, NE
	STR x8, [sp, 1304]
	LDR x8, [sp, 1304]
	CMP x8, 0
	B.NE label29
	B label_end30
label29:
	LDR x10, [sp, 1320]
	STR x10, [sp, 1296]
	LDR x8, [sp, 1336]
	LDR x9, [sp, 1320]
	SDIV x8, x8, x9
	STR x8, [sp, 1288]
	LDR x8, [sp, 1288]
	LDR x9, [sp, 1320]
	MUL x8, x8, x9
	STR x8, [sp, 1280]
	LDR x8, [sp, 1336]
	LDR x9, [sp, 1280]
	SUB x8, x8, x9
	STR x8, [sp, 1320]
	LDR x10, [sp, 1296]
	STR x10, [sp, 1336]
	B label28while
label_end30:
	LDR x8, [sp, 1680]
	LDR x9, [sp, 1336]
	EOR x8, x8, x9
	STR x8, [sp, 1272]
	MOVZ x8, 0xffff
	MOVK x8, 0x00, LSL 16
	STR x8, [sp, 1264]
	LDR x8, [sp, 1272]
	LDR x9, [sp, 1264]
	AND x8, x8, x9
	STR x8, [sp, 1680]
	MOV x8, 1
	STR x8, [sp, 1256]
	MOV x8, 1
	STR x8, [sp, 1248]
	MOV x8, 0
	STR x8, [sp, 1240]
label33while:
	MOV x8, 20
	STR x8, [sp, 1232]
	LDR x8, [sp, 1240]
	LDR x9, [sp, 1232]
	CMP x8, x9
	CSET x8, LT
	STR x8, [sp, 1224]
	LDR x8, [sp, 1224]
	CMP x8, 0
	B.NE label34
	B label_end35
label34:
	LDR x8, [sp, 1256]
	LDR x9, [sp, 1248]
	ADD x8, x8, x9
	STR x8, [sp, 1296]
	LDR x10, [sp, 1248]
	STR x10, [sp, 1256]
	LDR x10, [sp, 1296]
	STR x10, [sp, 1248]
	MOV x8, 1
	STR x8, [sp, 1216]
	LDR x8, [sp, 1240]
	LDR x9, [sp, 1216]
	ADD x8, x8, x9
	STR x8, [sp, 1240]
	B label33while
label_end35:
	LDR x8, [sp, 1680]
	LDR x9, [sp, 1248]
	ADD x8, x8, x9
	STR x8, [sp, 1208]
	MOVZ x8, 0xffff
	MOVK x8, 0x00, LSL 16
	STR x8, [sp, 1200]
	LDR x8, [sp, 1208]
	LDR x9, [sp, 1200]
	AND x8, x8, x9
	STR x8, [sp, 1680]
	MOV x8, 2
	STR x8, [sp, 1192]
	LDR x8, [sp, 1672]
	LDR x9, [sp, 1192]
	ADD x8, x8, x9
	STR x8, [sp, 1184]
	MOV x8, 1
	STR x8, [sp, 1176]
	MOV x8, 2
	STR x8, [sp, 1168]
label38while:
	MOV x8, 2
	STR x8, [sp, 1160]
	LDR x8, [sp, 1184]
	LDR x9, [sp, 1160]
	SDIV x8, x8, x9
	STR x8, [sp, 1152]
	MOV x8, 1
	STR x8, [sp, 1144]
	LDR x8, [sp, 1152]
	LDR x9, [sp, 1144]
	ADD x8, x8, x9
	STR x8, [sp, 1136]
	LDR x8, [sp, 1168]
	LDR x9, [sp, 1136]
	CMP x8, x9
	CSET x8, LT
	STR x8, [sp, 1128]
	LDR x8, [sp, 1128]
	CMP x8, 0
	B.NE label39
	B label_end40
label39:
	LDR x8, [sp, 1184]
	LDR x9, [sp, 1168]
	SDIV x8, x8, x9
	STR x8, [sp, 1120]
	LDR x8, [sp, 1120]
	LDR x9, [sp, 1168]
	MUL x8, x8, x9
	STR x8, [sp, 1112]
	LDR x8, [sp, 1184]
	LDR x9, [sp, 1112]
	SUB x8, x8, x9
	STR x8, [sp, 1104]
	MOV x8, 0
	STR x8, [sp, 1096]
	LDR x8, [sp, 1104]
	LDR x9, [sp, 1096]
	CMP x8, x9
	CSET x8, EQ
	STR x8, [sp, 1088]
	LDR x8, [sp, 1088]
	CMP x8, 0
	B.NE label_if41
	B label_else42
label_if41:
	MOV x8, 0
	STR x8, [sp, 1080]
	LDR x10, [sp, 1080]
	STR x10, [sp, 1176]
	B label_end43
label_else42:
	B label_end43
label_end43:
	MOV x8, 1
	STR x8, [sp, 1072]
	LDR x8, [sp, 1168]
	LDR x9, [sp, 1072]
	ADD x8, x8, x9
	STR x8, [sp, 1168]
	B label38while
label_end40:
	LDR x8, [sp, 1176]
	CMP x8, 0
	B.NE label_if44
	B label_else45
label_if44:
	MOV x8, 37
	STR x8, [sp, 1064]
	LDR x8, [sp, 1680]
	LDR x9, [sp, 1064]
	MUL x8, x8, x9
	STR x8, [sp, 1056]
	LDR x8, [sp, 1056]
	LDR x9, [sp, 1184]
	ADD x8, x8, x9
	STR x8, [sp, 1048]
	MOVZ x8, 0xffff
	MOVK x8, 0x00, LSL 16
	STR x8, [sp, 1040]
	LDR x8, [sp, 1048]
	LDR x9, [sp, 1040]
	AND x8, x8, x9
	STR x8, [sp, 1680]
	B label_end46
label_else45:
	LDR x8, [sp, 1680]
	LDR x9, [sp, 1184]
	ADD x8, x8, x9
	STR x8, [sp, 1032]
	MOVZ x8, 0xffff
	MOVK x8, 0x00, LSL 16
	STR x8, [sp, 1024]
	LDR x8, [sp, 1032]
	LDR x9, [sp, 1024]
	AND x8, x8, x9
	STR x8, [sp, 1680]
	B label_end46
label_end46:
	MOV x8, 1
	STR x8, [sp, 1016]
	LDR x8, [sp, 1672]
	LDR x9, [sp, 1016]
	ADD x8, x8, x9
	STR x8, [sp, 1672]
	B label2while
label_end4:
	MOV x8, 255
	STR x8, [sp, 1008]
	LDR x8, [sp, 1680]
	LDR x9, [sp, 1008]
	AND x8, x8, x9
	STR x8, [sp, 1000]
	LDR x0, [sp, 1000]
	MOV x16, 1
	ADD sp, sp, 1696
	BL  _exit
