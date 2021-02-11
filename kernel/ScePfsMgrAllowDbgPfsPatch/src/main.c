/*
 * ScePfsMgr debug patch
 * Copyright (C) 2021 Princess of Sleeping
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include <psp2kern/kernel/modulemgr.h>
#include <taihen.h>

tai_hook_ref_t ksceSblAimgrIsCEX_ref;
int ksceSblAimgrIsCEX_patch(void){

	TAI_CONTINUE(int, ksceSblAimgrIsCEX_ref);

	return 0;
}

#define HookImport(module_name, library_nid, func_nid, func_name) \
	taiHookFunctionImportForKernel(0x10005, &func_name ## _ref, module_name, library_nid, func_nid, func_name ## _patch)

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp){

	HookImport("ScePfsMgr", 0xFD00C69A, 0xD78B04A2, ksceSblAimgrIsCEX);

	return SCE_KERNEL_START_SUCCESS;
}
