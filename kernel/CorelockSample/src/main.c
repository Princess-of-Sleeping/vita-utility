/*
 * Corelock sample
 * Copyright (C) 2020 Princess of Sleeping
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/threadmgr.h>
#include <psp2kern/kernel/cpu.h>
#include <psp2kern/kernel/debug.h>

SceCorelockContext corelock_ctx;

int sceCorelockThread(SceSize args, void *argp){

	int this_cpu_core = ksceKernelCpuGetCpuId();

	ksceDebugPrintf("[%d] sceCorelockThread 0x%X\n", this_cpu_core, ksceKernelGetThreadId());

	if(this_cpu_core == 1){
		ksceDebugPrintf("[%d] waiting 1000 usec\n", this_cpu_core);
		ksceKernelDelayThread(1000);
		ksceDebugPrintf("[%d] after 1000 usec\n", this_cpu_core);
	}

	ksceDebugPrintf("[%d] invoke sceKernelCorelockLock\n", this_cpu_core);
	ksceKernelCorelockLock(&corelock_ctx, 0); // Cores other than core0 cannot execute the code below unless core0 call ksceKernelCorelockUnlock
	ksceDebugPrintf("[%d] after  sceKernelCorelockLock\n", this_cpu_core);

	if(this_cpu_core == 0){
		ksceDebugPrintf("[%d] waiting 5 second\n", this_cpu_core);
		ksceKernelDelayThread(5 * 1000 * 1000);
		ksceDebugPrintf("[%d] after 5 second\n", this_cpu_core);
	}

	ksceDebugPrintf("[%d] invoke sceKernelCorelockUnlock\n", this_cpu_core);
	ksceKernelCorelockUnlock(&corelock_ctx);
	ksceDebugPrintf("[%d] after  sceKernelCorelockUnlock\n", this_cpu_core);

	ksceDebugPrintf("[%d] Corelock time %lld [usec]\n", this_cpu_core, ksceKernelGetSystemTimeWide());
	ksceDebugPrintf("[%d] sceCorelockThread exit\n", this_cpu_core);

	return 0;
}

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp){

	ksceDebugPrintf("[%d] module_start\n", ksceKernelCpuGetCpuId());

	if(ksceKernelCpuGetCpuId() != 0){
		ksceDebugPrintf("Make sure the module_start call is core0\n");
		return SCE_KERNEL_START_NO_RESIDENT;
	}

	ksceKernelCorelockInitialize(&corelock_ctx);

	SceUID thid_core1, thid_core2, thid_core3;

	thid_core1 = ksceKernelCreateThread("SceSyncThreadCore1", sceCorelockThread, 0x10000100, 0x1000, 0, 1 << 1, NULL);
	thid_core2 = ksceKernelCreateThread("SceSyncThreadCore2", sceCorelockThread, 0x10000100, 0x1000, 0, 1 << 2, NULL);
	thid_core3 = ksceKernelCreateThread("SceSyncThreadCore3", sceCorelockThread, 0x10000100, 0x1000, 0, 1 << 3, NULL);

	ksceDebugPrintf("thid_core1 : 0x%X\n", thid_core1);
	ksceDebugPrintf("thid_core2 : 0x%X\n", thid_core2);
	ksceDebugPrintf("thid_core3 : 0x%X\n", thid_core3);

	ksceKernelStartThread(thid_core1, 0, NULL);
	ksceKernelStartThread(thid_core2, 0, NULL);
	ksceKernelStartThread(thid_core3, 0, NULL);
	sceCorelockThread(0, NULL);

	return SCE_KERNEL_START_SUCCESS;
}
