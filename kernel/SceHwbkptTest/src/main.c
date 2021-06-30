/*
 * Hardware break point test
 * Copyright (C) 2021 Princess of Sleeping
 */

#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/threadmgr.h>
#include <psp2kern/kernel/proc_event.h>
#include <psp2kern/kernel/sysclib.h>
#include <psp2kern/kernel/sysroot.h>
#include <psp2kern/kernel/debug.h>
#include <psp2kern/kernel/dipsw.h>
#include <psp2kern/sblaimgr.h>
#include <taihen.h>

#define HookImport(module_name, library_nid, func_nid, func_name) \
	taiHookFunctionImportForKernel(0x10005, &func_name ## _ref, module_name, library_nid, func_nid, func_name ## _patch)

int module_get_offset(SceUID pid, SceUID modid, int segidx, uint32_t offset, uintptr_t *dst);
int module_get_export_func(SceUID pid, const char *modname, uint32_t libnid, uint32_t funcnid, uintptr_t *func);

int (* sceKernelSetTHBP)(SceUID thid, SceUInt32 vis_level, void *BVR, SceUInt32 BCR);
int (* sceKernelGetTHBP)(SceUID thid, SceUInt32 vis_level, void **BVR, SceUInt32 *BCR);

int proc_create(SceUID pid, SceProcEventInvokeParam2 *a2, int a3){

	char titleid[0x20];

	ksceKernelSysrootGetProcessTitleId(pid, titleid, sizeof(titleid));

	if(0 == strcmp(titleid, "NPXS10015") && 0 != ksceSblAimgrIsTool()){

		SceUID thread_id = ksceKernelGetProcessMainThread(pid);
		SceUID module_id = ksceKernelGetProcessMainModule(pid);

		SceKernelModuleInfo module_info;
		memset(&module_info, 0, sizeof(module_info));
		module_info.size = sizeof(module_info);

		ksceKernelGetModuleInfo(pid, module_id, &module_info);

		SceUInt32 BCR = 0;

		BCR |= (1      & 1);            // BE:break point enable
		BCR |= (0b11   & 0b11)   << 1;  // Any world (user, privileged, svc)
		BCR |= (0b1111 & 0b1111) << 5;  // Byte address select (i think byte mask?)
		BCR |= (0b01   & 0b11)   << 14; // Secure state access control
		BCR |= (0b000  & 0b111)  << 20; // M:VA match

		void *bkpt_address = NULL;

		// module_start + push + sp ctrl only
		if(0 != ksceSblAimgrIsTool()){
			bkpt_address = module_info.segments[0].vaddr + 0x4900 + 8;
		}else if(0 != ksceSblAimgrIsCEX()){
			bkpt_address = module_info.segments[0].vaddr + 0x484c + 8;
		}

		if(bkpt_address != NULL){
			sceKernelSetTHBP(thread_id, 5, bkpt_address, BCR);
		}else{
			ksceDebugPrintf("[hwbkpt] Not supported device\n");
		}
	}

	return 0;
}

const SceProcEventHandler handler = {
	.size           = sizeof(SceProcEventHandler),
	.create         = proc_create,
	.exit           = NULL,
	.kill           = NULL,
	.stop           = NULL,
	.start          = NULL,
	.switch_process = NULL
};

SceUID proc_event_guid = -1;

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp){

	if(0 == ksceKernelCheckDipsw(0xE4)){
		ksceDebugPrintf("Dipsw 0xE4 must be set\n");
		return SCE_KERNEL_START_NO_RESIDENT;
	}

	int res;

	res = module_get_export_func(0x10005, "SceKernelThreadMgr", 0xe2c40624, 0x385831A1, (uintptr_t *)&sceKernelSetTHBP);
	if(res < 0){
		ksceDebugPrintf("%s:L%-4d:%s\n", __FILE__, __LINE__, __FUNCTION__);
		return SCE_KERNEL_START_NO_RESIDENT;
	}

	res = module_get_export_func(0x10005, "SceKernelThreadMgr", 0xe2c40624, 0x453B764A, (uintptr_t *)&sceKernelGetTHBP);
	if(res < 0){
		ksceDebugPrintf("%s:L%-4d:%s\n", __FILE__, __LINE__, __FUNCTION__);
		return SCE_KERNEL_START_NO_RESIDENT;
	}

	res = ksceKernelRegisterProcEventHandler("SceArmHwbkpt", &handler, 0);
	if(res < 0){
		ksceDebugPrintf("%s:L%-4d:%s\n", __FILE__, __LINE__, __FUNCTION__);
		return SCE_KERNEL_START_NO_RESIDENT;
	}

	proc_event_guid = res;

	return SCE_KERNEL_START_SUCCESS;
}
