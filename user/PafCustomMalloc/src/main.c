/*
 * ScePaf Custom Malloc
 * Copyright (C) 2020 Princess of Sleeping
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/clib.h>
#include <psp2/sysmodule.h>
#include "paf.h"

typedef struct SceSysmoduleOpt {
	int flags;
	int *result;
	int unused[2];
} SceSysmoduleOpt;

char test_heap[0x2000] __attribute__((aligned(0x20)));

int main(int argc, char *argv[]){

	void *res;
	int load_res;
	ScePafInit init_param;
	ScePafHeapInfo heap_info;
	SceSysmoduleOpt sysmodule_opt;

	sceClibMemset(&heap_info, 0, sizeof(heap_info));

	init_param.global_heap_size = 0x8000;
	init_param.a2               = 0xFFFFFFFF;
	init_param.a3               = 0xFFFFFFFF;
	init_param.use_gxm          = 0;
	init_param.heap_opt_param1  = 1;
	init_param.heap_opt_param2  = 1;

	load_res = 0xFFFFFFFF;
	sysmodule_opt.flags  = 0x10; // with arg
	sysmodule_opt.result = &load_res;

	sceSysmoduleLoadModuleInternalWithArg(SCE_SYSMODULE_INTERNAL_PAF, sizeof(init_param), &init_param, &sysmodule_opt);

	res = scePafHeapInit(&heap_info, test_heap, sizeof(test_heap), "ScePafHeap", NULL);

	sceClibPrintf("scePafHeapInit : 0x%X\n", res);
	sceClibPrintf("heap_info ptr  : 0x%X\n", &heap_info);
	sceClibPrintf("test_heap ptr  : 0x%X\n", test_heap);
	sceClibPrintf("heap_info.a3   : 0x%X\n", heap_info.a3);

	SceSize list[] = {0x4000, 0x200, 0x400, 0x800};
	void *ptr_list[sizeof(list) / sizeof(SceSize)];

	for(int i=0;i<(sizeof(list) / sizeof(SceSize));i++){
		ptr_list[i] = scePafMallocWithInfo(&heap_info, list[i]);

		ptr_list[i] = scePafReallocWithInfo(&heap_info, ptr_list[i], list[i] + 0x123);

		sceClibPrintf("malloc test    : 0x%08X/0x%X\n", ptr_list[i], list[i]);

		scePafFreeWithInfo(&heap_info, ptr_list[i]);



		ptr_list[i] = scePafMallocAlignWithInfo(&heap_info, 0x400, list[i]);

		sceClibPrintf("malloc test    : 0x%08X/0x%X(with align)\n", ptr_list[i], list[i]);
	}

	for(int i=0;i<(sizeof(list) / sizeof(SceSize));i++){
		scePafFreeWithInfo(&heap_info, ptr_list[i]);
		ptr_list[i] = NULL;
	}

	scePafHeapFini(&heap_info);

	sceSysmoduleUnloadModuleInternal(SCE_SYSMODULE_INTERNAL_PAF);

	sceKernelExitProcess(0);

	return 0;
}
