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
#include <psp2/paf.h>
#include "paf.h"

char test_heap[0x8000] __attribute__((aligned(0x20)));

int main(int argc, char *argv[]){

	int load_res;
	ScePafInitParam init_param;
	ScePafHeapContext heap_context;
	SceSysmoduleOpt sysmodule_opt;

	sceClibMemset(&heap_context, 0, sizeof(heap_context));

	init_param.global_heap_size  = 0x8000;
	init_param.unk_0x04          = 0xFFFFFFFF;
	init_param.unk_0x08          = 0xFFFFFFFF;
	init_param.is_cdialog_mode   = 0;
	init_param.heap_option_align = 0x4 << 8;
	init_param.heap_option_unk   = 1;

	load_res = 0xFFFFFFFF;
	sysmodule_opt.flags  = 0x10; // with arg
	sysmodule_opt.result = &load_res;

	sceSysmoduleLoadModuleInternalWithArg(SCE_SYSMODULE_INTERNAL_PAF, sizeof(init_param), &init_param, &sysmodule_opt);

	scePafCreateHeap(&heap_context, test_heap, sizeof(test_heap), "ScePafHeap", NULL);

	sceClibPrintf("test_heap ptr  : 0x%X\n", test_heap);

	SceSize list[] = {0x10000, 0x200, 0x400, 0x800, 0x100, 0x80, 0x40, 0x20, 0x10};
	void *ptr_list[sizeof(list) / sizeof(SceSize)];

	for(int i=0;i<(sizeof(list) / sizeof(SceSize));i++){
		ptr_list[i] = scePafMallocWithContext(&heap_context, list[i]);

		ptr_list[i] = scePafReallocWithContext(&heap_context, ptr_list[i], list[i] + 0x123);

		sceClibPrintf("malloc test    : 0x%08X/0x%X\n", ptr_list[i], list[i]);

		scePafFreeWithContext(&heap_context, ptr_list[i]);



		ptr_list[i] = scePafMallocAlignWithContext(&heap_context, 0x400, list[i]);

		sceClibPrintf("malloc test    : 0x%08X/0x%X(with align)\n", ptr_list[i], list[i]);
	}

	for(int i=0;i<(sizeof(list) / sizeof(SceSize));i++){
		scePafFreeWithContext(&heap_context, ptr_list[i]);
		ptr_list[i] = NULL;
	}

	scePafDeleteHeap(&heap_context);

	sceSysmoduleUnloadModuleInternal(SCE_SYSMODULE_INTERNAL_PAF);

	sceKernelExitProcess(0);

	return 0;
}
