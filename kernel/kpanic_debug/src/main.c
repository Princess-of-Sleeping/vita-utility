/*
 * PSVita kpanic_debug
 * Copyright (C) 2021, Princess of Sleeping
 */

#include <psp2kern/kernel/modulemgr.h>
#include <taihen.h>

const char qaf_patch[] = {
	0x01, 0x20, 0x00, 0xBF
};

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp) {

	SceUID SceKernelModulemgr_modid = ksceKernelSearchModuleByName("SceKernelModulemgr");

	/*
	 * Switch dbgFingerprint display to module name display
	 */
	taiInjectDataForKernel(0x10005, SceKernelModulemgr_modid, 0, 0xA73E, qaf_patch, 4);

	SceUID SceBsod_moduleid = ksceKernelSearchModuleByName("SceKernelBlueScreenOfDeath");
	if(SceBsod_moduleid > 0){
		SceKernelModuleInfo info;
		info.size = sizeof(info);
		ksceKernelGetModuleInfo(0x10005, SceBsod_moduleid, &info);

		*(char *)(info.segments[1].vaddr + 8) |= 1;
	}

	return SCE_KERNEL_START_SUCCESS;
}
