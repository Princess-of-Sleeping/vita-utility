/*
 * Cpu core sync sample
 * Copyright (C) 2020 Princess of Sleeping
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/threadmgr.h>
#include <psp2kern/kernel/cpu.h>
#include <psp2kern/kernel/sysmem.h>
#include <taihen.h>

#define GetExport(modname, libnid, funcnid, func) module_get_export_func(0x10005, modname, libnid, funcnid, (uintptr_t *)func)

int module_get_export_func(SceUID pid, const char *modname, uint32_t libnid, uint32_t funcnid, uintptr_t *func);

#define SCE_CPU_WAIT_CORE_0 1
#define SCE_CPU_WAIT_CORE_1 2
#define SCE_CPU_WAIT_CORE_2 3
#define SCE_CPU_WAIT_CORE_3 0

typedef struct SceCpuCoreSyncCtx {
	int lock;
	int16_t core_count;
	int16_t last_wait_core; // 0:core3, 1:core0, 2:core1, 3:core2
} SceCpuCoreSyncCtx;

void (* sceKernelInitCpuCoreSyncCtx)(SceCpuCoreSyncCtx *pCtx);

void (* sceKernelCoreSyncAll)(SceCpuCoreSyncCtx *ctx);
void (* sceKernelCoreSyncWait)(SceCpuCoreSyncCtx *ctx, int core);

SceCpuCoreSyncCtx cpu_sync_ctx;

int sceSyncThread(SceSize args, void *argp){

	ksceDebugPrintf("[%d] sceSyncThread 0x%X\n", ksceKernelCpuGetCpuId(), ksceKernelGetThreadId());

	if(ksceKernelCpuGetCpuId() == 2){
		sceKernelCoreSyncWait(&cpu_sync_ctx, SCE_CPU_WAIT_CORE_3);
	}

	ksceDebugPrintf("[%d] after sceKernelCoreSyncWait\n", ksceKernelCpuGetCpuId());

	if(ksceKernelCpuGetCpuId() == 3){
		ksceKernelDelayThread(5 * 1000 * 1000);
	}

	ksceDebugPrintf("[%d] invoke sceKernelCoreSyncAll\n", ksceKernelCpuGetCpuId());

	sceKernelCoreSyncAll(&cpu_sync_ctx);

	ksceDebugPrintf("[%d] sceSyncThread exit\n", ksceKernelCpuGetCpuId());

	return 0;
}

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp){

	ksceDebugPrintf("[%d] module_start\n", ksceKernelCpuGetCpuId());

	if(ksceKernelCpuGetCpuId() != 0){
		ksceDebugPrintf("Make sure the module_start call is core0\n");
		return SCE_KERNEL_START_NO_RESIDENT;
	}

	GetExport("SceSysmem", 0x54BF2BAB, 0x4CD4D921, &sceKernelInitCpuCoreSyncCtx);
	GetExport("SceSysmem", 0x54BF2BAB, 0xA5C9DBBA, &sceKernelCoreSyncAll);
	GetExport("SceSysmem", 0x54BF2BAB, 0x9D72DD1B, &sceKernelCoreSyncWait);

	sceKernelInitCpuCoreSyncCtx(&cpu_sync_ctx);

	SceUID thid_core1, thid_core2, thid_core3;

	thid_core1 = ksceKernelCreateThread("SceSyncThreadCore1", sceSyncThread, 0x10000100, 0x2000, 0, 1 << 1, NULL);
	thid_core2 = ksceKernelCreateThread("SceSyncThreadCore2", sceSyncThread, 0x10000100, 0x2000, 0, 1 << 2, NULL);
	thid_core3 = ksceKernelCreateThread("SceSyncThreadCore3", sceSyncThread, 0x10000100, 0x2000, 0, 1 << 3, NULL);

	ksceDebugPrintf("thid_core1 : 0x%X\n", thid_core1);
	ksceDebugPrintf("thid_core2 : 0x%X\n", thid_core2);
	ksceDebugPrintf("thid_core3 : 0x%X\n", thid_core3);

	ksceKernelStartThread(thid_core1, 0, NULL);
	ksceKernelStartThread(thid_core2, 0, NULL);
	ksceKernelStartThread(thid_core3, 0, NULL);

	sceKernelCoreSyncAll(&cpu_sync_ctx);

	return SCE_KERNEL_START_SUCCESS;
}
