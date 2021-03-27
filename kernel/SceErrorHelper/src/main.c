/*
 * SceErrorHelper
 * Copyright (C) 2021 Princess of Sleeping
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/sysclib.h>
#include <psp2kern/kernel/sysmem.h>
#include <taihen.h>

typedef struct SceErrorStrings {
	char s[0x10];
} SceErrorStrings;

tai_hook_ref_t _sceErrorGetExternalString_ref;
int _sceErrorGetExternalString_patch(SceErrorStrings *error_strings, int error_code){

	int res = TAI_CONTINUE(int, _sceErrorGetExternalString_ref, error_strings, error_code);
	if(res >= 0){
		SceErrorStrings errstr_kern;
		memset(&errstr_kern, 0, sizeof(errstr_kern));
		snprintf(errstr_kern.s, sizeof(errstr_kern.s), "0x%X", error_code);

		res = ksceKernelMemcpyKernelToUser((uintptr_t)error_strings, &errstr_kern, sizeof(errstr_kern));
	}

	return res;
}

#define HookExport(module_name, library_nid, func_nid, func_name) \
	taiHookFunctionExportForKernel(0x10005, &func_name ## _ref, module_name, library_nid, func_nid, func_name ## _patch)

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp){

	HookExport("SceError", 0x5CD2CAD1, 0x85747003, _sceErrorGetExternalString);

	return SCE_KERNEL_START_SUCCESS;
}
