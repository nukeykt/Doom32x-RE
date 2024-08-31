.text
.global _R_DrawTexture
_R_DrawTexture:
	mov.l	r8,@-r15
	ldc	r5,gbr
	mov.l	r9,@-r15
	mov.l	r10,@-r15
	mov.l	r11,@-r15
	mov.l	r12,@-r15
	mov.l	r13,@-r15
	mov.l	r14,@-r15
	mov.l	@(84,gbr),r0
	mov	r0,r10
	mov.l	@(92,gbr),r0
	mov	r0,r6
	mov.l	@(4,r5),r2
L1:
	mov.l	@(8,gbr),r0
	cmp/gt	r0,r2
	bf	L2
	bra	L10
	nop
L2:
	mov	r1,r3
	shar	r3
	shar	r3
	shar	r3
	shar	r3
	shar	r3
	shar	r3
	shar	r3
	add	r6,r1
	mov.l	@V5,r0
	cmp/gt	r0,r3
	bf	L3
	mov	r0,r3
L3:
	mov.l	@V11,r14
	mov.l	r3,@r14
	mov.l	@V6,r0
	mov.l	r0,@(4,r14)
	mov	r2,r0
	shll2	r0
	mov.l	@V7,r11
	mov.l	@(r0,r11),r8
	mov.l	@(96,gbr),r0
	add	r0,r8
	shlr16	r8
	shlr2	r8
	shlr	r8
	mov		r8,r0
	shll2	r0
	mov.l	@V8,r11
	mov.l	@(r0,r11),r10
	mov.l	@(104,gbr),r0
	dmuls.l	r10,r0
	sts	macl,r9
	sts	mach,r0
	xtrct	r0,r9
	mov.l	@(100,gbr),r0
	neg	r9,r9
	add	r0,r9
	shlr16	r9
	mov.l	@(12,r4),r11
	mul.l	r3,r11
	sts	macl,r11
	swap.w	r11,r10
	shal	r0
	shlr16	r11
	exts.w	r11,r11
	addc	r11,r11
	mov	#72,r10
	sub	r11,r10
	mov	r2,r0
	shll	r0
	mov.l	@V9,r11
	mov.w	@(r0,r11),r0
	cmp/gt	r0,r10
	bt	L4
	mov	r0,r10
	add	1,r10
L4:
	mov.l	@(16,r4),r11
	mul.l	r3,r11
	sts	macl,r11
	swap.w	r11,r0
	shal	r0
	shlr16	r11
	exts.w	r11,r11
	addc	r11,r11
	mov	#71,r12
	sub	r11,r12
	mov.l	@(28,r14),r7
	mov.l	@V10,r11
	mov	r2,r0
	shll	r0
	mov.w	@(r0,r11),r0
	cmp/ge	r0,r12
	bf	L5
	mov	r0,r12
	add	-1,r12
L5:
	cmp/gt	r12,r10
	bt L9
	mov.l	@(20,r4),r13
	mov	#72,r0
	sub	r10,r0
	mul.l	r0,r7
	sts	macl,r0
	sub	r0,r13
	mov.l	@(8,r4),r0
	shll16	r0
L6:
	cmp/pl	r13
	bt	L7
	add	-1,r9
	bra	L6
	add	r0,r13
L7:
	mov.l	@(4,r4),r0
	add	-1,r0
	and	r0,r9
	mov.l	@(8,r4),r0
	mul.l	r9,r0
	sts	macl,r0
	shll16	r0
	add	r0,r13
	mov.l	@V2,r5
	mul.l	r10,r5
	sts	macl,r9
	add	r2,r9
	add r2,r9
	mov.l	@V4,r3
	add	r3,r9
	mov.l	@V1,r0
	mov.l	@r0,r0
	mov.l	@V3,r8
	add	r8,r0
	mov.l	@r4,r3
	sub	r10,r12
	add	1,r12
	mov	r13,r8
	shlr16	r8
	add	r8,r3
	shll16	r13
	mov	r7,r10
	shlr16	r10
	shll16	r7
	clrt
	nop
L8:
	mov.b	@r3,r8
	addc	r7,r13
	addc	r10,r3
	shll	r8
	mov.w	@(r0,r8),r8
	dt	r12
	mov.w	r8,@r9
	bf/s	L8
	add	r5,r9
L9:
	bra	L1
	add	1,r2
L10:
	mov.l	@r15,r14
	mov.l	@r15,r13
	mov.l	@r15,r12
	mov.l	@r15,r11
	mov.l	@r15,r10
	mov.l	@r15,r9
	mov.l	@r15,r8
	rts
	nop

V1:
	.long	_DAT_060089c0
V2:
	.long	320
V3:
	.long	256
V4:
	.long	040018A0h
V5:
	.long	00007FFFh
V6:
	.long	02000000h
V7:
	.long	_xtoviewangle
V8:
	.long	_finetangent
V9:
	.long	_clipboundtop
V10:
	.long	_clipboundbottom
V11:
	.long	FFFFFF00h

.global	_R_DrawColumnASM
_R_DrawColumnASM:
	mov.l	r8,@-r15
	mov.l	r9,@-r15
	mov.w	@r4+,r5
	mov.w	@r4+,r6
	mov.w	@V18,r7
	mul.l	r6,r7
	sts	macl,r2
	add	r5,r2
	mov.l	V12,r3
	add	r3,r2
	mov.w	@r4+,r6
	mov.l	@r4+,r0
	mov.l	@V15,r8
	add	r8,r0
	mov.l	@r4+,r1
	mov.l	@r4+,r3
	mov.l	@r4+,r5
	mov	r3,r8
	shlr16	r8
	add	r8,r1
	shll16	r3
	mov	r5,r4
	shlr16	r5
	shll16	r4
	clrt
	nop
L11:
	mov.b	@r1,r8
	addc	r4,r3
	addc	r5,r1
	shll	r8
	mov.w	@(r0,r8),r8
	dt
	mov.w	r8,@r2
	bf/s	L11
	add	r7,r2
	mov.l	@r15+,r9
	mov.l	@r15+,r8
	rts
	nop
	.short 0
.global	_R_DrawSpanASM
_R_DrawSpanASM:
	mov.l	r8,@-r15
	mov.l	r9,@-r15
	mov.l	r10,@-r15
	mov.l	r11,@-r15
	mov.w	@r4+,r5
	mov.w	@r4+,r6
	mov.w	@V18,r7
	mul.l	r6,r7
	sts	macl,r7
	add	r5,r2
	mov.l	@V12,r3
	add	r3,r2
	mov.w	@r4+,r6
	add	r6,r2
	mov.l	@r4+,r3
	add	r6,r2
	mov.l	@V15,r8
	add	r8,r3
	mov.l	@r4+,r1
	mov.l	@r4+,r7
	mov.l	@r4+,r8
	mov.l	@r4+,r9
	mov.l	@r4+,r10
	shll8	r9
	mov.l	@V16,r4
	shlr2	r9
	shll8	r10
	mov.l	@V17,r5
	shlr2	r10
	swap.w	r9,r11
L12:
	swap.w	r7,r0
	and	r4,r0
	and	r5,r11
	or	r11,r0
	mov.b	@(r0,r1),r0
	add	r8,r7
	add	r10,r9
	shll	r0
	mov.w	@(r0,r3),r0
	dt	r6
	mov.w	r0,@-r2
	bf/s	L12
	swap.w	r9,r11
	mov.l	@r15+,r11
	mov.l	@r15+,r10
	mov.l	@r15+,r9
	mov.l	@r15+,r8
	rts
	nop
	.short 0
	.long 0
	.long 0
	.long 0
V12:
	.long	040018A0h
V13:
	.long	20000000h
V14:
	.long	000000FFh
V15:
	.long	00000100h
V16:
	.long	0000003Fh
V17:
	.long	00000FC0h
V18:
	.long	320


.global	_R_MapPlane
_R_MapPlane:
	mov.l	r8,@-r15
	mov	r4,r6
	mov.l	r9,@-r15
	mov	r4,r3
	mov.l	r10,@-r15
	mov	r4,r2
	mov.l	r11,@-r15
	shlr16	r3
	mov.l	r12,@-r15
	shlr8	r2
	mov.l	@V19,r10
	mov.w	@V37,r0
	mov.l	@V20,r9
	and	r0,r6
	and	r0,r2
	mov	r2,r0
	mov.l	@r10,r10
	shll	r0
	mov.w	@(r0,r9),r9
	sub	r3,r6
	mulu.w	r10,r9
	neg	r6,r6
	add	1,r6
	sts	macl,r8
	shlr8	r8
	shlr8	r8
	mov.l	@V21,r5
	shlr2	r8
	mov	r3,r0
	shll	r0
	mov.w	@(r0,r5),r5
	shlr2	r8
	mov	r3,r0
	shll	r0
	mov.w	@(r0,r5),r5
	mulu.w	r8,r5
	sts	macl,r5
	shlr8	r5
	mov.l	@V23,r12
	shlr2	r5
	mov.l	@V22,r11
	shlr2	r5
	mov	r3,r0
	shll2	r0
	mov.l	@(r0,r11),r11
	add	r11,r12
	mov	r12,r0
	shlr16	r0
	mov.l	@V24,r12
	mov.l	@r12,r12
	shlr2	r0
	mov.l	@V26,r11
	shlr	r0
	mov.l	@r11,r11
	shll2	r0
	mov.l	@(r0,r11),r11
	shar	r11
	muls.w	r11,r5
	sts	macl,r9
	shar	r9
	shar	r9
	shar	r9
	shar	r9
	add	r12,r9
	mov.l	@V25,r7
	mov.l	@r7,r7
	shll2	r7
	shll2	r7
	mov.l	@V26,r11
	mov.l	@(r0,r11),r11
	shar	r11
	muls.w	r11,r5
	sts	macl,r12
	sub	r12,r7
	shll2	r7
	mov.l	@V28,r1
	mov.l	@r1,r1
	mov.l	@V29,r12
	mov.l	@r12,r12
	muls.w	r8,r1
	sts	macl,r10
	shar	r10
	muls.w	r8,r12
	shar	r10
	sts	macl,r8
	shll2	r8
	mov.w	@V34,r12
	mulu.w	r2,r12
	shar	r10
	shar	r10
	sts	macl,r2
	add	r3,r2
	mov.l	@V30,r12
	add	r12,r2
	add	r3,r2
	add	2,r2
	mov.l	@V35,r3
	mov.l	@r3,r3
	mov.w	@V38,r12
	add	r12,r3
	mov.l	@V36,r1
	mov.l	@r1,r1
	mov.w	@V31,r4
	mov.w	@V32,r5
	swap.w	r7,r11
	nop
L13:
	swap.w	r9,r0
	and	r4,r0
	and	r5,r11
	or	r11,r10
	mov.b	@(r0,r1),r0
	add	r10,r9
	add	r8,r7
	shll	r0
	mov.w	@(r0,r3),r0
	dt	r6
	mov.w	r0,@-r2
	bf/s	L13
	swap.w	r7,r11
	mov.l	@r15+,r12
	mov.l	@r15+,r11
	mov.l	@r15+,r10
	mov.l	@r15+,r9
	mov.l	@r15+,r8
	rts
	nop

	.long	0
	.long	0

V19:
	.long	_planelight
V20:
	.long	_yslope
V21:
	.long	_distscale
V22:
	.long	_xtoviewangle
V23:
	.long	_planeangle
V24:
	.long	_planex
V25:
	.long	_planey
V26:
	.long	_finesine
V27:
	.long	_finecosine
V28:
	.long	_basexscale
V29:
	.long	_baseyscale
V30:
	.long	040018A0h
V31:
	.short	003fh
V32:
	.short	0fc0h
V33:
	.short	0
V34:
	.short	0140h
V35:
	.long	_DAT_060089c0
V36:
	.long	_planesource
V37:
	.short	00ffh
V38:
	.short	0100h
