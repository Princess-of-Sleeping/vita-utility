/*
 * SceFios2Kernel tester
 * Copyright (C) 2021, Princess of Sleeping
 */

#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/threadmgr.h>
#include <psp2kern/kernel/debug.h>
#include <taihen.h>

#define HookExport(module_name, library_nid, func_nid, func_name) taiHookFunctionExportForKernel(SCE_GUID_KERNEL_PROCESS_ID, &func_name ## _ref, module_name, library_nid, func_nid, func_name ## _patch)
#define HookImport(module_name, library_nid, func_nid, func_name) taiHookFunctionImportForKernel(SCE_GUID_KERNEL_PROCESS_ID, &func_name ## _ref, module_name, library_nid, func_nid, func_name ## _patch)
#define HookOffset(modid, offset, thumb, func_name) taiHookFunctionOffsetForKernel(SCE_GUID_KERNEL_PROCESS_ID, &func_name ## _ref, modid, 0, offset, thumb, func_name ## _patch)

#define HookRelease(hook_uid, hook_func_name)({ \
	(hook_uid > 0) ? taiHookReleaseForKernel(hook_uid, hook_func_name ## _ref) : -1; \
})

#define GetExport(modname, lib_nid, func_nid, func) module_get_export_func(SCE_GUID_KERNEL_PROCESS_ID, modname, lib_nid, func_nid, (uintptr_t *)func)

int module_get_offset(SceUID pid, SceUID modid, int segidx, size_t offset, uintptr_t *addr);
int module_get_export_func(SceUID pid, const char *modname, SceNID libnid, SceNID funcnid, uintptr_t *func);

int sce_fios2kernel_test_main(void *argp){

	int res;
	SceSSize (* sceFios2KernelSanitizePath)(char *dst, const char *src, int n);
	tai_module_info_t tai_info;

	tai_info.size = sizeof(tai_module_info_t);

	res = taiGetModuleInfoForKernel(SCE_GUID_KERNEL_PROCESS_ID, "SceFios2Kernel", &tai_info);
	if(res < 0)
		return res;

	switch(tai_info.module_nid){
	case 0x10ECF2D0: // 3.60
		module_get_offset(SCE_GUID_KERNEL_PROCESS_ID, tai_info.modid, 0, 0x5dc | 1, (uintptr_t *)&sceFios2KernelSanitizePath);
	break;
	default:
		ksceDebugPrintf("Unknown SceFios2Kernel fingerprint : 0x%08X\n", tai_info.module_nid);
		return -1;
	}

	char resolved_path[0x400];

	res = sceFios2KernelSanitizePath(resolved_path, "host0:data/./test/kernel/memory/../build/", sizeof(resolved_path));

	ksceDebugPrintf("resolve res : 0x%X\n", res);
	ksceDebugPrintf("%s\n", resolved_path); // host0:data/test/kernel/build

	return 0;
}

void _start() __attribute__ ((weak, alias ("module_start")));
int module_start(SceSize argc, const void *args){

	ksceKernelRunWithStack(0x2000, sce_fios2kernel_test_main, NULL);

	return SCE_KERNEL_START_NO_RESIDENT;
}
