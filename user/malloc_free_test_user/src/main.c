/*
 * PS Vita malloc/free test
 * Copyright (C) 2021 Princess of Sleeping
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include <psp2/kernel/modulemgr.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/clib.h>
#include <psp2/appmgr.h>
#include <psp2/sysmodule.h>
#include <psp2/paf.h>

const char    sceUserMainThreadName[]          = "alloc_free_test_user";
const int     sceUserMainThreadPriority        = 0x40;
const int     sceUserMainThreadCpuAffinityMask = 1 << 0;
const SceSize sceUserMainThreadStackSize       = 0x40000;

const int sceKernelPreloadModuleInhibit = SCE_KERNEL_PRELOAD_INHIBIT_LIBC
					| SCE_KERNEL_PRELOAD_INHIBIT_LIBDBG
					| SCE_KERNEL_PRELOAD_INHIBIT_APPUTIL
					| SCE_KERNEL_PRELOAD_INHIBIT_LIBSCEFT2
					| SCE_KERNEL_PRELOAD_INHIBIT_LIBPERF;

typedef struct AllocFreeTestParam {
	SceSize length;
	SceUInt32 align;
} __attribute__ ((__packed__)) AllocFreeTestParam;

const AllocFreeTestParam allocFreeTestParam[] = {
	{
		.length = 0x10,
		.align  = 4
	},
	{
		.length = 0x16,
		.align  = 8
	},
	{
		.length = 0x20,
		.align  = 0x10
	},
	{
		.length = 0x80,
		.align  = 0x20
	},
	{
		.length = 0x60,
		.align  = 0x40
	},
	{
		.length = 0x50,
		.align  = 0x80
	},
	{
		.length = 0x90,
		.align  = 0x100
	},
	{
		.length = 0x110,
		.align  = 0x200
	},
	{
		.length = 0x1A0,
		.align  = 0x400
	},
	{
		.length = 0x250,
		.align  = 0x800
	},
	{
		.length = 0x250,
		.align  = 0x880
	},
	{
		.length = 0x20,
		.align  = 0x80
	},
	{
		.length = 0x58,
		.align  = 0x4
	},
	{
		.length = 0x4,
		.align  = 0x4
	},
	{
		.length = 0x16,
		.align  = 0x4
	},
	{
		.length = 0x1,
		.align  = 0x8
	},
	{
		.length = 0x20,
		.align  = 0x10
	},
	{
		.length = 0x80,
		.align  = 0x20
	},
	{
		.length = 0x60,
		.align  = 0x40
	},
	{
		.length = 0x50,
		.align  = 0x80
	},
	{
		.length = 0x90,
		.align  = 0x100
	},
	{
		.length = 0x110,
		.align  = 0x200
	},
	{
		.length = 0x1A0,
		.align  = 0x400
	},
	{
		.length = 0x250,
		.align  = 0x800
	},
	{
		.length = 0x250,
		.align  = 0x880
	},
	{
		.length = 0x20,
		.align  = 0x80
	},
	{
		.length = 0x58,
		.align  = 0x4
	},
	{
		.length = 0x4,
		.align  = 0x4
	},
	{
		.length = 0x16,
		.align  = 0x4
	},
	{
		.length = 0x1,
		.align  = 0x8
	},
	{
		.length = 0x20,
		.align  = 0x10
	},
	{
		.length = 0x80,
		.align  = 0x20
	},
	{
		.length = 0x60,
		.align  = 0x40
	},
	{
		.length = 0x50,
		.align  = 0x80
	},
	{
		.length = 0x90,
		.align  = 0x100
	},
	{
		.length = 0x110,
		.align  = 0x200
	},
	{
		.length = 0x1A0,
		.align  = 0x400
	},
	{
		.length = 0x250,
		.align  = 0x800
	},
	{
		.length = 0x250,
		.align  = 0x880
	},
	{
		.length = 0x20,
		.align  = 0x80
	},
	{
		.length = 0x58,
		.align  = 0x4
	},
	{
		.length = 0x4,
		.align  = 0x4
	},
	{
		.length = 0x16,
		.align  = 0x4
	},
	{
		.length = 0x1,
		.align  = 0x8
	},
	{
		.length = 0x20,
		.align  = 0x10
	},
	{
		.length = 0x80,
		.align  = 0x20
	},
	{
		.length = 0x60,
		.align  = 0x40
	},
	{
		.length = 0x50,
		.align  = 0x80
	},
	{
		.length = 0x90,
		.align  = 0x100
	},
	{
		.length = 0x110,
		.align  = 0x200
	},
	{
		.length = 0x1A0,
		.align  = 0x400
	},
	{
		.length = 0x250,
		.align  = 0x800
	},
	{
		.length = 0x250,
		.align  = 0x880
	},
	{
		.length = 0x20,
		.align  = 0x80
	},
	{
		.length = 0x58,
		.align  = 0x4
	},
	{
		.length = 0x4,
		.align  = 0x4
	},
	{
		.length = 0x16,
		.align  = 0x4
	},
	{
		.length = 0x1,
		.align  = 0x8
	},
	{
		.length = 0x20,
		.align  = 0x10
	},
	{
		.length = 0x80,
		.align  = 0x20
	},
	{
		.length = 0x60,
		.align  = 0x40
	},
	{
		.length = 0x50,
		.align  = 0x80
	},
	{
		.length = 0x90,
		.align  = 0x100
	},
	{
		.length = 0x110,
		.align  = 0x200
	},
	{
		.length = 0x1A0,
		.align  = 0x400
	},
	{
		.length = 0x250,
		.align  = 0x800
	},
	{
		.length = 0x250,
		.align  = 0x880
	},
	{
		.length = 0x20,
		.align  = 0x80
	},
	{
		.length = 0x58,
		.align  = 0x4
	},
	{
		.length = 0x4,
		.align  = 0x4
	},
	{
		.length = 0x16,
		.align  = 0x4
	},
	{
		.length = 0x1,
		.align  = 0x8
	},
	{
		.length = 0x20,
		.align  = 0x10
	},
	{
		.length = 0x80,
		.align  = 0x20
	},
	{
		.length = 0x60,
		.align  = 0x40
	},
	{
		.length = 0x50,
		.align  = 0x80
	},
	{
		.length = 0x90,
		.align  = 0x100
	},
	{
		.length = 0x110,
		.align  = 0x200
	},
	{
		.length = 0x1A0,
		.align  = 0x400
	},
	{
		.length = 0x250,
		.align  = 0x800
	},
	{
		.length = 0x250,
		.align  = 0x880
	},
	{
		.length = 0x20,
		.align  = 0x80
	},
	{
		.length = 0x58,
		.align  = 0x4
	},
	{
		.length = 0x4,
		.align  = 0x4
	},
	{
		.length = 0x16,
		.align  = 0x4
	},
	{
		.length = 0x1,
		.align  = 0x8
	},
	{
		.length = 0x20,
		.align  = 0x10
	},
	{
		.length = 0x80,
		.align  = 0x20
	},
	{
		.length = 0x60,
		.align  = 0x40
	},
	{
		.length = 0x50,
		.align  = 0x80
	},
	{
		.length = 0x90,
		.align  = 0x100
	},
	{
		.length = 0x110,
		.align  = 0x200
	},
	{
		.length = 0x1A0,
		.align  = 0x400
	},
	{
		.length = 0x250,
		.align  = 0x800
	},
	{
		.length = 0x250,
		.align  = 0x880
	},
	{
		.length = 0x20,
		.align  = 0x80
	},
	{
		.length = 0x58,
		.align  = 0x4
	},
	{
		.length = 0x4,
		.align  = 0x4
	},
	{
		.length = 0x16,
		.align  = 0x4
	},
	{
		.length = 0x1,
		.align  = 0x8
	},
	{
		.length = 0x20,
		.align  = 0x10
	},
	{
		.length = 0x80,
		.align  = 0x20
	},
	{
		.length = 0x60,
		.align  = 0x40
	},
	{
		.length = 0x50,
		.align  = 0x80
	},
	{
		.length = 0x90,
		.align  = 0x100
	},
	{
		.length = 0x110,
		.align  = 0x200
	},
	{
		.length = 0x1A0,
		.align  = 0x400
	},
	{
		.length = 0x250,
		.align  = 0x800
	},
	{
		.length = 0x250,
		.align  = 0x880
	},
	{
		.length = 0x20,
		.align  = 0x80
	},
	{
		.length = 0x58,
		.align  = 0x4
	},
	{
		.length = 0x4,
		.align  = 0x4
	},
	{
		.length = 0x16,
		.align  = 0x4
	},
	{
		.length = 0x1,
		.align  = 0x8
	},
	{
		.length = 0x20,
		.align  = 0x10
	},
	{
		.length = 0x80,
		.align  = 0x20
	},
	{
		.length = 0x60,
		.align  = 0x40
	},
	{
		.length = 0x50,
		.align  = 0x80
	},
	{
		.length = 0x90,
		.align  = 0x100
	},
	{
		.length = 0x110,
		.align  = 0x200
	},
	{
		.length = 0x1A0,
		.align  = 0x400
	},
	{
		.length = 0x250,
		.align  = 0x800
	},
	{
		.length = 0x250,
		.align  = 0x880
	},
	{
		.length = 0x20,
		.align  = 0x80
	},
	{
		.length = 0x58,
		.align  = 0x4
	},
	{
		.length = 0x4,
		.align  = 0x4
	},
	{
		.length = 0x16,
		.align  = 0x4
	},
	{
		.length = 0x1,
		.align  = 0x8
	},
	{
		.length = 0x20,
		.align  = 0x10
	},
	{
		.length = 0x80,
		.align  = 0x20
	},
	{
		.length = 0x60,
		.align  = 0x40
	},
	{
		.length = 0x50,
		.align  = 0x80
	},
	{
		.length = 0x90,
		.align  = 0x100
	},
	{
		.length = 0x110,
		.align  = 0x200
	},
	{
		.length = 0x1A0,
		.align  = 0x400
	},
	{
		.length = 0x250,
		.align  = 0x800
	},
	{
		.length = 0x250,
		.align  = 0x880
	},
	{
		.length = 0x20,
		.align  = 0x80
	},
	{
		.length = 0x58,
		.align  = 0x4
	},
	{
		.length = 0x4,
		.align  = 0x4
	},
	{
		.length = 0x16,
		.align  = 0x4
	},
	{
		.length = 0x1,
		.align  = 0x8
	},
	{
		.length = 0x20,
		.align  = 0x10
	},
	{
		.length = 0x80,
		.align  = 0x20
	},
	{
		.length = 0x60,
		.align  = 0x40
	},
	{
		.length = 0x50,
		.align  = 0x80
	},
	{
		.length = 0x90,
		.align  = 0x100
	},
	{
		.length = 0x110,
		.align  = 0x200
	},
	{
		.length = 0x1A0,
		.align  = 0x400
	},
	{
		.length = 0x250,
		.align  = 0x800
	},
	{
		.length = 0x250,
		.align  = 0x880
	},
	{
		.length = 0x20,
		.align  = 0x80
	},
	{
		.length = 0x58,
		.align  = 0x4
	},
	{
		.length = 0x4,
		.align  = 0x4
	},
	{
		.length = 0x16,
		.align  = 0x4
	},
	{
		.length = 0x1,
		.align  = 0x8
	},
	{
		.length = 0x20,
		.align  = 0x10
	},
	{
		.length = 0x80,
		.align  = 0x20
	},
	{
		.length = 0x60,
		.align  = 0x40
	},
	{
		.length = 0x50,
		.align  = 0x80
	},
	{
		.length = 0x90,
		.align  = 0x100
	},
	{
		.length = 0x110,
		.align  = 0x200
	},
	{
		.length = 0x1A0,
		.align  = 0x400
	},
	{
		.length = 0x250,
		.align  = 0x800
	},
	{
		.length = 0x250,
		.align  = 0x880
	},
	{
		.length = 0x20,
		.align  = 0x80
	},
	{
		.length = 0x58,
		.align  = 0x4
	},
	{
		.length = 0x4,
		.align  = 0x4
	},
	{
		.length = 0x16,
		.align  = 0x4
	},
	{
		.length = 0x1,
		.align  = 0x8
	},
	{
		.length = 0x20,
		.align  = 0x10
	},
	{
		.length = 0x80,
		.align  = 0x20
	},
	{
		.length = 0x60,
		.align  = 0x40
	},
	{
		.length = 0x50,
		.align  = 0x80
	},
	{
		.length = 0x90,
		.align  = 0x100
	},
	{
		.length = 0x110,
		.align  = 0x200
	},
	{
		.length = 0x1A0,
		.align  = 0x400
	},
	{
		.length = 0x250,
		.align  = 0x800
	},
	{
		.length = 0x250,
		.align  = 0x880
	},
	{
		.length = 0x20,
		.align  = 0x80
	},
	{
		.length = 0x58,
		.align  = 0x4
	},
	{
		.length = 0x4,
		.align  = 0x4
	},
	{
		.length = 0x16,
		.align  = 0x4
	},
	{
		.length = 0x1,
		.align  = 0x8
	}
};

#define ALLOC_FREE_PARAM_NUM (sizeof(allocFreeTestParam) / sizeof(allocFreeTestParam[0]))

typedef struct ScePafInit { // size is 0x18
	SceSize global_heap_size;
	int a2;
	int a3;
	int use_gxm;
	int heap_opt_param1;
	int heap_opt_param2;
} ScePafInit;

int load_paf(void){

	int load_res;
	ScePafInit init_param;
	SceSysmoduleOpt sysmodule_opt;

	init_param.global_heap_size = 0x100000;
	init_param.a2               = 0xFFFFFFFF;
	init_param.a3               = 0xFFFFFFFF;
	init_param.use_gxm          = 0;
	init_param.heap_opt_param1  = 1;
	init_param.heap_opt_param2  = 1;

	load_res = 0xFFFFFFFF;
	sysmodule_opt.flags  = 0x10; // with arg
	sysmodule_opt.result = &load_res;

	return sceSysmoduleLoadModuleInternalWithArg(SCE_SYSMODULE_INTERNAL_PAF, sizeof(init_param), &init_param, &sysmodule_opt);
}

int alloc_free_test_paf(void){

	void *pResult[ALLOC_FREE_PARAM_NUM];
	SceInt64 time_s, time_e;

	sceClibPrintf("%s\n", __FUNCTION__);

	for(int i=0;i<ALLOC_FREE_PARAM_NUM;i++){
		time_s = sceKernelGetSystemTimeWide();
		pResult[i] = sce_paf_private_malloc(allocFreeTestParam[i].length);
		time_e = sceKernelGetSystemTimeWide();

		sceClibPrintf("%02d %p length=0x%08X %10lld usec\n", i, pResult[i], allocFreeTestParam[i].length, time_e - time_s);
	}

	for(int i=0;i<ALLOC_FREE_PARAM_NUM;i++){

		time_s = sceKernelGetSystemTimeWide();
		sce_paf_private_free(pResult[i]);
		time_e = sceKernelGetSystemTimeWide();

		sceClibPrintf("%02d %p %10lld usec\n", i, pResult[i], time_e - time_s);
		pResult[i] = NULL;
	}

	for(int i=0;i<ALLOC_FREE_PARAM_NUM;i++){

		time_s = sceKernelGetSystemTimeWide();
		pResult[i] = sce_paf_memalign(allocFreeTestParam[i].align, allocFreeTestParam[i].length);
		time_e = sceKernelGetSystemTimeWide();

		sceClibPrintf("%02d %p align=0x%08X length=0x%08X %10lld usec\n", i, pResult[i], allocFreeTestParam[i].align, allocFreeTestParam[i].length, time_e - time_s);
	}

	for(int i=0;i<ALLOC_FREE_PARAM_NUM;i++){

		time_s = sceKernelGetSystemTimeWide();
		sce_paf_private_free(pResult[i]);
		time_e = sceKernelGetSystemTimeWide();

		sceClibPrintf("%02d %p %10lld usec\n", i, pResult[i], time_e - time_s);
		pResult[i] = NULL;
	}

	return 0;
}

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp){

	int res;

	res = load_paf();
	if(res < 0){
		sceClibPrintf("load_paf failed 0x%X\n", res);
	}

	alloc_free_test_paf();

	sceAppMgrDestroyAppByAppId(~2);

	return SCE_KERNEL_START_SUCCESS;
}
