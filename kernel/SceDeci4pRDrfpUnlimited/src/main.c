/*
 * Unlimited SceDeci4pRDrfp io file control patch
 * Copyright (C) 2021 Princess of Sleeping
 */

#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/threadmgr.h>
#include <psp2kern/kernel/debug.h>
#include <psp2kern/io/fcntl.h>
#include <psp2kern/io/stat.h>
#include <taihen.h>

#define HookImport(module_name, library_nid, func_nid, func_name) \
	taiHookFunctionImportForKernel(0x10005, &func_name ## _ref, module_name, library_nid, func_nid, func_name ## _patch)
#define HookOffset(modid, offset, thumb, func_name) \
	taiHookFunctionOffsetForKernel(0x10005, &func_name ## _ref, modid, 0, offset, thumb, func_name ## _patch)

#define EXEC_WITH_KERN_CONTEXT(stmt)                        \
do {                                                        \
	int __old_pid__  = ksceKernelSetProcessId(0x10005); \
	int __old_perm__ = ksceKernelSetPermission(0x80);   \
	stmt;                                               \
	ksceKernelSetProcessId(__old_pid__);                \
	ksceKernelSetPermission(__old_perm__);              \
} while (0)

tai_hook_ref_t sceDeci4pRDrfpDriveCheck_ref;
int sceDeci4pRDrfpDriveCheck_patch(const char *path, int a2, int a3, int a4){
	return 0;
}

tai_hook_ref_t ksceIoGetstat_ref;
int ksceIoGetstat_patch(const char *file, SceIoStat *stat){

	int res;

	res = TAI_CONTINUE(int, ksceIoGetstat_ref, file, stat);
	if(res == 0x80010002){
		EXEC_WITH_KERN_CONTEXT(
			res = TAI_CONTINUE(int, ksceIoGetstat_ref, file, stat);
		);
	}

	return res;
}

tai_hook_ref_t ksceIoGetstatByFd_ref;
int ksceIoGetstatByFd_patch(SceUID fd, SceIoStat *stat){

	int res;

	res = TAI_CONTINUE(int, ksceIoGetstatByFd_ref, fd, stat);
	if(res == 0x8001000D){
		EXEC_WITH_KERN_CONTEXT(
			res = TAI_CONTINUE(int, ksceIoGetstatByFd_ref, fd, stat);
		);
	}

	return res;
}

tai_hook_ref_t ksceIoOpen_ref;
SceUID ksceIoOpen_patch(const char *file, int flags, SceMode mode){

	int res;

	res = TAI_CONTINUE(SceUID, ksceIoOpen_ref, file, flags, mode);
	if(res == 0x80010002){
		EXEC_WITH_KERN_CONTEXT(
			res = TAI_CONTINUE(int, ksceIoOpen_ref, file, flags, mode);
		);
	}

	return res;
}

tai_hook_ref_t ksceIoPread_ref;
int ksceIoPread_patch(SceUID fd, void *data, SceSize size, SceOff offset){

	int res;

	res = TAI_CONTINUE(int, ksceIoPread_ref, fd, data, size, offset);
	if(res == 0x8001000D){
		EXEC_WITH_KERN_CONTEXT(
			res = TAI_CONTINUE(int, ksceIoPread_ref, fd, data, size, offset);
		);
	}

	return res;
}

tai_hook_ref_t ksceIoDopen_ref;
SceUID ksceIoDopen_patch(const char *dirname){

	int res;

	res = TAI_CONTINUE(SceUID, ksceIoDopen_ref, dirname);
	if(res == 0x80010002){
		EXEC_WITH_KERN_CONTEXT(
			res = TAI_CONTINUE(int, ksceIoDopen_ref, dirname);
		);
	}

	return res;
}

tai_hook_ref_t ksceIoDread_ref;
int ksceIoDread_patch(SceUID fd, SceIoDirent *dir){

	int res;

	EXEC_WITH_KERN_CONTEXT(
		res = TAI_CONTINUE(int, ksceIoDread_ref, fd, dir);
	);

	return res;
}

SceUID hookid[7];

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp){

	SceUID module_id = ksceKernelSearchModuleByName("SceDeci4pRDrfp");
	if(module_id < 0)
		return SCE_KERNEL_START_NO_RESIDENT;

	hookid[0] = HookOffset(module_id, 0x1024, 1, sceDeci4pRDrfpDriveCheck);
	hookid[1] = HookImport("SceDeci4pRDrfp", 0x40fd29c7, 0x75192972, ksceIoOpen);
	hookid[2] = HookImport("SceDeci4pRDrfp", 0x40fd29c7, 0x2a17515d, ksceIoPread);
	hookid[3] = HookImport("SceDeci4pRDrfp", 0x40fd29c7, 0x463b25cc, ksceIoDopen);
	hookid[4] = HookImport("SceDeci4pRDrfp", 0x40fd29c7, 0x20cf5fc7, ksceIoDread);
	hookid[5] = HookImport("SceDeci4pRDrfp", 0x40fd29c7, 0x75c96d25, ksceIoGetstat);
	hookid[6] = HookImport("SceDeci4pRDrfp", 0x40fd29c7, 0x462f059b, ksceIoGetstatByFd);

	return SCE_KERNEL_START_SUCCESS;
}
