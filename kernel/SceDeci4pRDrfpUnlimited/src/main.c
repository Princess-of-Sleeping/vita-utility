/*
 * Unlimited SceDeci4pRDrfp io file control patch
 * Copyright (C) 2021 Princess of Sleeping
 */

#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/debug.h>
#include <taihen.h>

#define HookImport(module_name, library_nid, func_nid, func_name) \
	taiHookFunctionImportForKernel(0x10005, &func_name ## _ref, module_name, library_nid, func_nid, func_name ## _patch)
#define HookOffset(modid, offset, thumb, func_name) \
	taiHookFunctionOffsetForKernel(0x10005, &func_name ## _ref, modid, 0, offset, thumb, func_name ## _patch)

tai_hook_ref_t sceDeci4pRDrfpDriveCheck_ref;
int sceDeci4pRDrfpDriveCheck_patch(const char *path, int a2, int a3, int a4){
	return 0;
}

tai_hook_ref_t sceKernelSetProcessId_ref;
SceUID sceKernelSetProcessId_patch(SceUID pid){
	return TAI_CONTINUE(SceUID, sceKernelSetProcessId_ref, 0x10005);
}

tai_hook_ref_t sceKernelSetPermission_ref;
int sceKernelSetPermission_patch(int perm){
	return TAI_CONTINUE(int, sceKernelSetPermission_ref, 0x80);
}

SceUID hookid[3];

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp){

	SceUID module_id = ksceKernelSearchModuleByName("SceDeci4pRDrfp");
	if(module_id < 0)
		return SCE_KERNEL_START_NO_RESIDENT;

	hookid[0] = HookOffset(module_id, 0x1024, 1, sceDeci4pRDrfpDriveCheck);
	hookid[1] = HookImport("SceDeci4pRDrfp", 0xe2c40624, 0x0486f239, sceKernelSetProcessId);
	hookid[2] = HookImport("SceDeci4pRDrfp", 0xe2c40624, 0x02eedf17, sceKernelSetPermission);

	return SCE_KERNEL_START_SUCCESS;
}
