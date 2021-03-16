/*
 * SceDipsw show
 * Copyright (C) 2021 Princess of Sleeping
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/debug.h>
#include <psp2kern/kernel/dipsw.h>

void dipsw_test(SceUInt8 bit){
	ksceDebugPrintf("0x%X -> byte:0x%X/bit:0x%X", bit, (bit >> 5) << 2, 1 << (bit & 0x1F));
}

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp){

	for(int i=0x80;i<0x100;i++){
		ksceDebugPrintf("Dipsw 0x%X(%d):%d\n", i, i, ksceKernelCheckDipsw(i));
	}

	return SCE_KERNEL_START_NO_RESIDENT;
}
