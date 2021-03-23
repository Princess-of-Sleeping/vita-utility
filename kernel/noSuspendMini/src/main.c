/*
 * PS Vita noSuspend mini
 * Copyright (C) 2021 Princess of Sleeping
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/threadmgr.h>
#include <psp2kern/kernel/suspend.h>
#include <psp2kern/kernel/debug.h>

int sceKernelNoSuspendThread(SceSize args, void *argp){

	while(1){
		ksceKernelDelayThread(1 * 1000 * 1000);

		ksceKernelPowerTick(SCE_KERNEL_POWER_TICK_DISABLE_AUTO_SUSPEND);
	}

	return 0;
}

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp){

	int res;
	SceUID thid;

	thid = ksceKernelCreateThread("NoSuspendThread", sceKernelNoSuspendThread, 0x10000100, 0x1000, 0, 0, NULL);
	if(thid < 0){
		ksceDebugPrintf("sceKernelCreateThread failed : 0x%X\n", thid);
		return SCE_KERNEL_START_FAILED;
	}

	res = ksceKernelStartThread(thid, 0, NULL);
	if(res < 0){
		ksceDebugPrintf("sceKernelStartThread failed : 0x%X\n", res);
		return SCE_KERNEL_START_FAILED;
	}

	return SCE_KERNEL_START_SUCCESS;
}
