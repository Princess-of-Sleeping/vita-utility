/*
 * PSVita kernel qaf host0
 * Copyright (C) 2021, Princess of Sleeping
 */

#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/sysroot.h>

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp) {

	SceKblParam *pKblParam = ksceKernelSysrootGetKblParam();

	pKblParam->qa_flags[0xE] |= 1;

	return SCE_KERNEL_START_NO_RESIDENT;
}
