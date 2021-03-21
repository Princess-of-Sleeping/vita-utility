/*
 * PS Vita Devkit TV
 * Copyright (C) 2021 Princess of Sleeping
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/threadmgr.h>
#include <psp2kern/kernel/debug.h>
#include <taihen.h>

int ksceOledDisplayOff(void);
int ksceOledDisplayOn(void);

const char *const hdmi_msg[] = {"HDMI disconnection", "HDMI connected"};

int (* sceHdmiGetState)(int *state);
void (* scePowerDimmingCallback)(int a1, int a2, int a3);

int update_screen_with_state(int state){

	int res;

	if(state == 0){
		res = ksceOledDisplayOn();
		if(res >= 0)
			scePowerDimmingCallback(1, 0, 0);
	}else{
		res = ksceOledDisplayOff();
	}

	// ksceDebugPrintf("%s:%s\n", __FUNCTION__, hdmi_msg[state & 1]);

	return res;
}

int update_screen(void){

	int res, state;

	res = sceHdmiGetState(&state);
	if(res < 0){
		ksceDebugPrintf("Failed sceHdmiGetState\n");
		return res;
	}

	update_screen_with_state(state);

	return res;
}

tai_hook_ref_t sceHdmiSetState_ref;
int sceHdmiSetState_patch(int state){

	int res = TAI_CONTINUE(int, sceHdmiSetState_ref, state);
	if(res == 0)
		update_screen();

	return res;
}

int screen_update_thread(SceSize args, void *argp){

	ksceKernelDelayThread(5 * 1000 * 1000);

	update_screen();

	ksceKernelExitDeleteThread(0);
	return 0;
}

void scePowerDimmingCallbackHook(int a1, int a2, int a3){

	int res, state;

	res = sceHdmiGetState(&state);
	if(res < 0){
		ksceDebugPrintf("Failed sceHdmiGetState\n");
		return;
	}

	if(a1 == 1 && state == 0){
		scePowerDimmingCallback(a1, a2, a3);
	}
}

typedef void (* ScePowerIdleCallback)(int a1, int a2, int a3);

int kscePowerSetIdleCallback(int a1, int inhibit_reset, SceUInt64 time, ScePowerIdleCallback cb, int a6);

#define HookImport(module_name, library_nid, func_nid, func_name) \
	taiHookFunctionImportForKernel(0x10005, &func_name ## _ref, module_name, library_nid, func_nid, func_name ## _patch)
#define HookOffset(modid, offset, thumb, func_name) \
	taiHookFunctionOffsetForKernel(0x10005, &func_name ## _ref, modid, 0, offset, thumb, func_name ## _patch)

int module_get_offset(SceUID pid, SceUID modid, int segidx, uint32_t offset, uintptr_t *dst);

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp){

	int res;

	SceUID module_id;

	module_id = ksceKernelSearchModuleByName("ScePower");
	if(module_id < 0){
		ksceDebugPrintf("Failed found %s\n", "ScePower");
		return SCE_KERNEL_START_FAILED;
	}

	res = module_get_offset(0x10005, module_id, 0, 0x667C | 1, (uintptr_t *)&scePowerDimmingCallback);
	if(res < 0){
		ksceDebugPrintf("Failed get scePowerDimmingCallback\n");
		return SCE_KERNEL_START_FAILED;
	}

	module_id = ksceKernelSearchModuleByName("SceHdmi");
	if(module_id < 0){
		ksceDebugPrintf("Failed found %s\n", "SceHdmi");
		return SCE_KERNEL_START_FAILED;
	}

	res = module_get_offset(0x10005, module_id, 0, 0x5234 | 1, (uintptr_t *)&sceHdmiGetState);
	if(res < 0){
		ksceDebugPrintf("Failed get SceHdmi internal function\n");
		return SCE_KERNEL_START_FAILED;
	}

	if(ksceKernelSearchModuleByName("SceSysStateMgr") >= 0){

		SceUID thid;
		thid = ksceKernelCreateThread("screen_update_thread", screen_update_thread, 0x10000100, 0x1000, 0, 0, NULL);
		if(thid < 0)
			return SCE_KERNEL_START_FAILED;

		res = ksceKernelStartThread(thid, 0, NULL);
		if(res < 0){
			ksceKernelDeleteThread(thid);
			return SCE_KERNEL_START_FAILED;
		}
	}else{
		update_screen();
	}

	HookOffset(module_id, 0x5250, 1, sceHdmiSetState);

	kscePowerSetIdleCallback(1, 0, 0LL, NULL, 0); // clear
	kscePowerSetIdleCallback(1, 0x400, 45000000, scePowerDimmingCallbackHook, 0);

	return SCE_KERNEL_START_SUCCESS;
}
