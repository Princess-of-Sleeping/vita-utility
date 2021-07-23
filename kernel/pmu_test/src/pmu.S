
	.align 4
	.text
	.cpu cortex-a9
	.arch armv7-a
	.syntax unified
	.thumb
	.thumb_func
	.fpu neon

	.global pmu_start
	.type   pmu_start, %function

pmu_start:
	mrc p15, #0, r0, c9, c12, #0
	orr r0, r0, #0x1
	mcr p15, #0, r0, c9, c12, #0
	bx lr

	.global pmu_reset_counter
	.type   pmu_reset_counter, %function

pmu_reset_counter:
	mrc p15, #0, r0, c9, c12, #0
	orr r0, r0, #0x4
	mcr p15, #0, r0, c9, c12, #0
	bx lr

	.global pmu_reset_evnt_counter
	.type   pmu_reset_evnt_counter, %function

pmu_reset_evnt_counter:
	mrc p15, #0, r0, c9, c12, #0
	orr r0, r0, #0x2
	mcr p15, #0, r0, c9, c12, #0
	bx lr

	.global pmu_enable_counter
	.type   pmu_enable_counter, %function

pmu_enable_counter:
	mrc p15, #0, r0, c9, c12, #1
	orr r0, r0, #0x80000000
	mcr p15, #0, r0, c9, c12, #1
	bx lr

	.global pmu_enable_event0
	.type   pmu_enable_event0, %function

pmu_enable_event0:
	mrc p15, #0, r0, c9, c12, #1
	orr r0, r0, #0x1
	mcr p15, #0, r0, c9, c12, #1
	bx lr

	.global pmu_set_event0
	.type   pmu_set_event0, %function

pmu_set_event0:
	movs r1, #0x0
	mcr p15, #0, r1, c9, c12, #5
	mcr p15, #0, r0, c9, c13, #1
	bx lr

	.global pmu_read_counter
	.type   pmu_read_counter, %function

pmu_read_counter:
	mrc p15, #0, r0, c9, c13, #0
	bx lr

	.global pmu_read_event0
	.type   pmu_read_event0, %function

pmu_read_event0:
	movs r0, #0x0
	mcr p15, #0, r0, c9, c12, #5
	mrc p15, #0, r0, c9, c13, #2
	bx lr


	// PMUSERENR
sceKernelPmuAccessibleUser:
	mrc p15, #0, r0, c9, c14, #0
	orr r0, r0, #0x1
	mcr p15, #0, r0, c9, c14, #0



	.data
