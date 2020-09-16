/*
 * SceVfs mount PoC
 * Copyright (C) 2020 Princess of Sleeping
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

	.align 4
	.text
	.cpu cortex-a9
	.arch armv7-a
	.syntax unified
	.thumb
	.thumb_func
	.fpu neon

	.global my_memcpy
	.type   my_memcpy, %function

my_memcpy:
	push {r0, r4, lr}

	subs r2, #0x20
	blo loc_81000122
	adds r2, r2, r0

loc_810000E8:
	lsls r3, r0, #0x1e
	beq loc_810000FE

	ldrb r3, [r1], #1
	strb r3, [r0], #1
	b loc_810000E8

loc_810000F6:
	ldr ip, [r1], #4
	str ip, [r0], #4

loc_810000FE:
	lsls r3, r0, #0x1b
	bne loc_810000F6
	subs r2, r2, r0
	blo loc_81000122
	lsls r3, r1, #0x1e
	bne loc_81000146

loc_8100010A:
	ldm.w r1!, {r3, r4, ip, lr}
	pld [r1, #0xc0]
	vldmia r1!, {d0, d1}
	subs r2, #0x20
	stm.w r0!, {r3, r4, ip, lr}
	vstmia r0!, {d0, d1}
	bhs loc_8100010A

loc_81000122:
	adds r2, #0x1c
	blt loc_81000132

loc_81000126:
	ldr ip, [r1], #4
	subs r2, #4
	str ip, [r0], #4
	bge loc_81000126

loc_81000132:
	lsls r3, r2, #0x1f
	blo loc_8100013E
	ldrh r3, [r1], #2
	strh r3, [r0], #2

loc_8100013E:
	bpl loc_81000144
	ldrb r3, [r1]
	strb r3, [r0]

loc_81000144:
	pop {r0, r4, pc}

loc_81000146:
	vld1.8 {d0, d1, d2, d3}, [r1]!
	pld [r1, #0x80]
	subs r2, #0x20
	vstmia r0!, {d0, d1, d2, d3}
	bhs loc_81000146
	b loc_81000122
	nop.w
	nop.w


	.global sceKernelCpuDcacheCleanMVACRange
	.type   sceKernelCpuDcacheCleanMVACRange, %function

sceKernelCpuDcacheCleanMVACRange:

	adds r2, r0, r1
	subs r2, #1
	lsrs r2, r2, #5
	subs.w r2, r2, r0, lsr #5
	dsb sy

loc_0x81021706:
	mcr p15, #0, r0, c7, c10, #1
	adds r0, #0x20
	subs r2, #1
	bhs loc_0x81021706
	dsb sy
	bx lr


	.global sceKernelCpuIcacheInvalidateRange
	.type   sceKernelCpuIcacheInvalidateRange, %function

sceKernelCpuIcacheInvalidateRange:

	adds r2, r0, r1
	subs r2, #1
	lsrs r2, r2, #5
	subs.w r2, r2, r0, lsr #5

loc_81021696:
	mcr p15, #0, r0, c7, c5, #1
	adds r0, #0x20
	subs r2, #1
	bhs loc_81021696
	dsb sy
	bx lr

	.global sceKernelCpuDcacheWritebackInvalidateRange
	.type   sceKernelCpuDcacheWritebackInvalidateRange, %function

sceKernelCpuDcacheWritebackInvalidateRange:

	adds r2, r0, r1
	subs r2, #1
	lsrs r2, r2, #5
	subs.w r2, r2, r0, lsr #5
	dsb sy

loc_81021726:
	mcr p15, #0, r0, c7, c14, #1
	adds r0, #0x20
	subs r2, #1
	bhs loc_81021726
	dsb sy
	bx lr

	.type   sub_810232C0, %function

sub_810232C0:
	and r3, r0, #0x70
	cmp r3, #0x10
	beq loc_810232FE
	bls loc_810232EE
	cmp r3, #0x30
	beq loc_810232EA
	cmp r3, #0x50
	bne loc_810232F4
	ands r0, r0, #0xc
	beq loc_81023302
	movw r3, #0x2007
	movt r3, #0x8002
	cmp r0, #4
	ite ne
	movne r0, r3
	moveq r0, #0xd0
	bx lr

loc_810232EA:
	movs r0, #8
	bx lr

loc_810232EE:
	cbnz r3, loc_810232F4
	movs r0, #0x80
	bx lr

loc_810232F4:
	movw r0, #0x2007
	movt r0, #0x8002
	bx lr

loc_810232FE:
	movs r0, #2
	bx lr

loc_81023302:
	movs r0, #0x40
	bx lr




// get_paddr_with_mode
sub_81023308:
	push {r4, r5, r6, lr}
	mov r6, r3
	ldr r4, [sp, #0x10]
	mrc p15, #0, r3, c3, c0, #0
	lsls r1, r1, #0x1f
	bpl loc_81023364

loc_81023316:
	mcr p15, #0, r2, c7, c8, #0
	mrc p15, #0, r1, c7, c4, #0
	and r5, r0, #1
	bics r5, r1
	bne loc_81023336
	mcr p15, #0, r2, c7, c8, #1
	mrc p15, #0, r1, c7, c4, #0
	bic.w r5, r0, r1, lsl #1
	lsls r5, r5, #0x1e
	bpl loc_81023378

loc_81023336:
	mcr p15, #0, r3, c3, c0, #0
	cbz r4, loc_81023348
	mov r0, r1
	bl sub_810232C0
	cmp r0, #0
	str r0, [r4]
	blt loc_8102336E

loc_81023348:
	mov r4, r1
	ubfx r0, r1, #1, #1
	bfi r4, r2, #0, #0xc
	cbnz r0, loc_8102335A
	str r4, [r6]
	adds r0, #1
	pop {r4, r5, r6, pc}

loc_8102335A:
	bfi r4, r2, #0, #0x18
	adds r0, #1
	str r4, [r6]
	pop {r4, r5, r6, pc}

loc_81023364:
	mov.w r1, #0x55555555
	mcr p15, #0, r1, c3, c0, #0
	b loc_81023316

loc_8102336E:
	movs r3, #0
	str r3, [r6]

loc_81023372:
	movs r3, #0
	str r3, [r4]
	pop {r4, r5, r6, pc}

loc_81023378:
	mcr p15, #0, r2, c7, c8, #2
	mrc p15, #0, r1, c7, c4, #0
	bic.w r5, r0, r1, lsl #4
	lsls r5, r5, #0x1b
	bmi loc_81023336
	mcr p15, #0, r2, c7, c8, #3
	mrc p15, #0, r1, c7, c4, #0
	bic.w r0, r0, r1, lsl #5
	lsls r0, r0, #0x1a
	bmi loc_81023336
	mcr p15, #0, r3, c3, c0, #0
	movs r3, #0
	movw r0, #0x2007
	str r3, [r6]
	movt r0, #0x8002
	cmp r4, #0
	bne loc_81023372
	pop {r4, r5, r6, pc}
	nop


sceKernelCpuGetPaddr:
	push {lr}
	mov r3, r1
	sub sp, #0xc
	movs r1, #0
	mov r2, r0
	movs r0, #0x33
	str r1, [sp]
	bl sub_81023308
	add sp, #0xc
	pop {pc}

	.global sceKernelCpuDcacheWritebackRange
	.type   sceKernelCpuDcacheWritebackRange, %function

sceKernelCpuDcacheWritebackRange:
	push {r3, lr}
	movs r2, #0x20
	bl cpu_dcache_writeback_range
	pop {r3, pc}


	.global cpu_dcache_writeback_range
	.type   cpu_dcache_writeback_range, %function
// sub_81022C50
cpu_dcache_writeback_range:
	push.w {r4, r5, r6, r7, r8, lr}
	// movw r7, #0xa004
	// movt r7, #0x8102
	sub sp, #8
	mov r5, r1
	mov r8, lr
	// ldr r3, [r7]
	mov r4, r2
	mov r6, r0
	// str r3, [sp, #4]

	cbz r1, loc_81022CB0
	lsls r2, r4, #0x1f
	bmi loc_81022CBE

loc_81022C70:
	mov r0, r6
	mov r1, r5
	bl sceKernelCpuDcacheCleanMVACRange
	lsls r3, r4, #0x1a
	itt pl
	ubfxpl r4, r6, #0, #0xc
	rsbpl.w r4, r4, #0x1000
	bpl loc_81022C8E
	b loc_81022CB0

loc_81022C88:
	add r6, r4
	mov.w r4, #0x1000

loc_81022C8E:
	mov r0, r6
	mov r1, sp
	bl sceKernelCpuGetPaddr
	cmp r0, #0
	blt loc_81022CF0
	cmp r4, r5
	it hs
	movhs r4, r5
	ldr r0, [sp]
	mov r1, r4
	bl WriteL2CacheReg
	subs r5, r5, r4
	bne loc_81022C88
	bl WaitL2CacheReg

loc_81022CB0:
	// ldr r2, [sp, #4]
	// ldr r3, [r7]
	// cmp r2, r3
	// bne loc_81022CFE
	add sp, #8
	pop.w {r4, r5, r6, r7, r8, pc}

loc_81022CBE:
	add.w r3, r0, #-0x18000000
	cmp.w r3, #0x8000000
	blo loc_81022CE6
	add.w r0, r0, #-0x60000000
	add.w r3, r6, #-0x20000000
	cmp.w r3, #0x20000000
	it hs
	cmphs.w r0, #0x21000000
	ite hs
	movhs r0, #0
	movlo r0, #1
	cmp r0, #0
	beq loc_81022C70
	b loc_81022CB0

loc_81022CE6:
	bl sceKernelCpuIsVaddrMapped
	cmp r0, #0
	beq loc_81022C70
	b loc_81022CB0

loc_81022CF0:
	movw r0, #0xb150
	mov r1, r8
	movt r0, #0x8102
	bkpt #0
	// bl #0x81019d20 // sceDebugPrintKernelPanic

loc_81022CFE:
	bkpt #0
	// bl #0x8101b520 // __stack_chk_fail
	nop

// sub_81024014
WriteL2CacheReg:
	cbz r1, loc_81024038
	movw r2, #:lower16:SceL2CacheRegBase
	movt r2, #:upper16:SceL2CacheRegBase

	ldr r2, [r2]
	and r3, r0, #0x1f
	bic r0, r0, #0x1f
	adds r1, r1, r3

loc_8102402A:
	str.w r0, [r2, #0x7b0]
	adds r0, #0x20
	subs r1, #0x20
	bgt loc_8102402A
	dmb sy

loc_81024038:
	bx lr


// sub_81023FD0
WaitL2CacheReg:
	movw r0, #:lower16:SceL2CacheRegBase
	movt r0, #:upper16:SceL2CacheRegBase
	ldr r0, [r0]
	mov.w r1, #0
	str.w r1, [r0, #0x730]

loc_81023FE2:
	ldr.w r1, [r0, #0x730]
	tst.w r1, #1
	bne loc_81023FE2
	dmb sy
	bx lr

	.global sceKernelCpuIsVaddrMapped
	.type   sceKernelCpuIsVaddrMapped, %function

sceKernelCpuIsVaddrMapped:
	push {r3, lr}
	bl SceCpuForKernel_9B8173F4
	cmp r0, #8
	beq loc_81022C4C
	ble loc_81022C48
	cmp r0, #0x40
	beq loc_81022C4C
	cmp r0, #0x80
	beq loc_81022C4C

loc_81022C44:
	movs r0, #0
	pop {r3, pc}

loc_81022C48:
	cmp r0, #2
	bne loc_81022C44

loc_81022C4C:
	movs r0, #1
	pop {r3, pc}

	.global SceCpuForKernel_9B8173F4
	.type   SceCpuForKernel_9B8173F4, %function

SceCpuForKernel_9B8173F4:
	push {r3, lr}
	mrc p15, #0, r2, c3, c0, #0
	mov.w r3, #0x55555555
	mcr p15, #0, r3, c3, c0, #0
	mcr p15, #0, r0, c7, c8, #0
	mrc p15, #0, r3, c7, c4, #0
	lsls r1, r3, #0x1f
	bpl loc_81023B36
	mcr p15, #0, r0, c7, c8, #1
	mrc p15, #0, r3, c7, c4, #0
	lsls r1, r3, #0x1f
	bmi loc_81023B42

loc_81023B36:
	mcr p15, #0, r2, c3, c0, #0
	mov r0, r3
	bl sub_810232C0
	pop {r3, pc}

loc_81023B42:
	mcr p15, #0, r0, c7, c8, #2
	mrc p15, #0, r3, c7, c4, #0
	lsls r1, r3, #0x1f
	bpl loc_81023B36
	mcr p15, #0, r0, c7, c8, #3
	mrc p15, #0, r3, c7, c4, #0
	lsls r1, r3, #0x1f
	bpl loc_81023B36
	mcr p15, #0, r2, c3, c0, #0
	movw r0, #0x2007
	movt r0, #0x8002
	pop {r3, pc}


	.global sceKernelCpuIcacheAndL2WritebackInvalidateRange
	.type   sceKernelCpuIcacheAndL2WritebackInvalidateRange, %function

sceKernelCpuIcacheAndL2WritebackInvalidateRange:
	push.w {r4, r5, r6, r7, r8, sb, sl, lr}
	// movw r7, #0xa004
	// movt r7, #0x8102
	sub sp, #8
	mov r8, r1
	mov sl, lr
	// ldr r3, [r7]
	mov sb, r0
	// str r3, [sp, #4]

	cbz r1, loc_81022F3C
	ubfx r4, r0, #0, #0xc
	mov r6, r0
	rsb.w r4, r4, #0x1000
	mov r5, r1
	b loc_81022F12

loc_81022F0C:
	add r6, r4
	mov.w r4, #0x1000

loc_81022F12:
	mov r0, r6
	mov r1, sp
	bl sceKernelCpuGetPaddr
	cmp r0, #0
	blt loc_81022F4A
	cmp r4, r5
	it hs
	movhs r4, r5
	ldr r0, [sp]
	mov r1, r4
	bl sub_8102403C
	subs r5, r5, r4
	bne loc_81022F0C
	bl WaitL2CacheReg
	mov r0, sb
	mov r1, r8
	bl sceKernelCpuIcacheInvalidateRange

loc_81022F3C:
	// ldr r2, [sp, #4]
	// ldr r3, [r7]
	// cmp r2, r3
	// bne loc_81022F58
	add sp, #8
	pop.w {r4, r5, r6, r7, r8, sb, sl, pc}

loc_81022F4A:
	movw r0, #0xb138
	mov r1, sl
	movt r0, #0x8102
	bkpt #0
	// bl loc_81019d20 // ksceDebugPrintKernelPanic
loc_81022F58:
	bkpt #0
	// bl loc_8101b520 // __stack_chk_fail

	.type   sub_8102403C, %function

sub_8102403C:
	cbz r1, loc_81024060
	movw r2, #:lower16:SceL2CacheRegBase
	movt r2, #:upper16:SceL2CacheRegBase
	ldr r2, [r2]
	and r3, r0, #0x1f
	bic r0, r0, #0x1f
	adds r1, r1, r3

loc_81024052:
	str.w r0, [r2, #0x7f0]
	adds r0, #0x20
	subs r1, #0x20
	bgt loc_81024052
	dmb sy

loc_81024060:
	bx lr

	.data
