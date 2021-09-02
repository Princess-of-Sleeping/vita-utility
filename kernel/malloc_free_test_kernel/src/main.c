/*
 * PS Vita malloc/free test
 * Copyright (C) 2021 Princess of Sleeping
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/threadmgr.h>
#include <psp2kern/kernel/sysmem.h>
#include <psp2kern/kernel/sysclib.h>
#include <psp2kern/kernel/debug.h>
#include <taihen.h>

int module_get_offset(SceUID pid, SceUID modid, int segidx, uint32_t offset, uintptr_t *dst);
int module_get_export_func(SceUID pid, const char *modname, uint32_t lib_nid, uint32_t func_nid, uintptr_t *func);

#define GetExport(modname, lib_nid, func_nid, func) module_get_export_func(0x10005, modname, lib_nid, func_nid, (uintptr_t *)func)

/*
 * If this flag is set, the function can return NULL if no free memory is found.
 * If not, wait in the function until free memory is available
 */
#define SCE_NETPS_MALLOC_FLAG_NOT_SLEEP (1 << 0)

/*
 * If this flag is set, the function will call memset to clear the found free memory to zero and then return a pointer.
 */
#define SCE_NETPS_MALLOC_FLAG_MEM_CLEAR (1 << 1)

void *(* sceNetPsMalloc)(SceSize len, int flags, SceUInt32 align);
int (* sceNetPsFree)(void *ptr);

int get_netps_function(void){

	SceUID module_id;

	module_id = ksceKernelSearchModuleByName("SceNetPs");
	if(module_id < 0){
		ksceDebugPrintf("Failed found %s\n", "SceNetPs");
		return module_id;
	}

	module_get_offset(0x10005, module_id, 0, 0x56B4 | 1, (uintptr_t *)&sceNetPsMalloc);
	module_get_offset(0x10005, module_id, 0, 0x598C | 1, (uintptr_t *)&sceNetPsFree);

	return 0;
}

void *(* sceKernelAllocWithOpt)(SceSize len, SceAllocOpt *pOpt);

int get_sysmem_function(void){

	int res;

	res = GetExport("SceSysmem", 0x63A519E5, 0xA2CD1697, &sceKernelAllocWithOpt);
	if(res < 0){
		ksceDebugPrintf("Failed found %s\n", "SceSysmem");
		return res;
	}

	return 0;
}

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

void *pResult[ALLOC_FREE_PARAM_NUM];

int alloc_free_test_system_internal(void){

	SceInt64 time_s, time_e;

	ksceDebugPrintf("%s\n", __FUNCTION__);

	for(int i=0;i<ALLOC_FREE_PARAM_NUM;i++){

		time_s = ksceKernelGetSystemTimeWide();

		pResult[i] = ksceKernelAlloc(allocFreeTestParam[i].length);

		time_e = ksceKernelGetSystemTimeWide();

		ksceDebugPrintf("%02d %p length=0x%08X %10lld usec\n", i, pResult[i], allocFreeTestParam[i].length, time_e - time_s);
	}

	for(int i=0;i<ALLOC_FREE_PARAM_NUM;i++){

		time_s = ksceKernelGetSystemTimeWide();

		ksceKernelFree(pResult[i]);

		time_e = ksceKernelGetSystemTimeWide();

		ksceDebugPrintf("%02d %p %10lld usec\n", i, pResult[i], time_e - time_s);
		pResult[i] = NULL;
	}

	for(int i=0;i<ALLOC_FREE_PARAM_NUM;i++){

		SceAllocOpt opt;
		memset(&opt, 0, sizeof(opt));

		opt.size  = sizeof(opt);
		opt.align = allocFreeTestParam[i].align;

		time_s = ksceKernelGetSystemTimeWide();

		pResult[i] = sceKernelAllocWithOpt(allocFreeTestParam[i].length, &opt);

		time_e = ksceKernelGetSystemTimeWide();

		ksceDebugPrintf("%02d %p align=0x%08X length=0x%08X %10lld usec\n", i, pResult[i], allocFreeTestParam[i].align, allocFreeTestParam[i].length, time_e - time_s);
	}

	for(int i=0;i<ALLOC_FREE_PARAM_NUM;i++){

		time_s = ksceKernelGetSystemTimeWide();

		ksceKernelFree(pResult[i]);

		time_e = ksceKernelGetSystemTimeWide();

		ksceDebugPrintf("%02d %p %10lld usec\n", i, pResult[i], time_e - time_s);
		pResult[i] = NULL;
	}

	return 0;
}

int alloc_free_test_system_external(void){

	SceInt64 time_s, time_e;

	ksceDebugPrintf("%s\n", __FUNCTION__);

	for(int i=0;i<ALLOC_FREE_PARAM_NUM;i++){

		time_s = ksceKernelGetSystemTimeWide();

		pResult[i] = ksceKernelAllocHeapMemory(0x1000B, allocFreeTestParam[i].length);

		time_e = ksceKernelGetSystemTimeWide();

		ksceDebugPrintf("%02d %p length=0x%08X %10lld usec\n", i, pResult[i], allocFreeTestParam[i].length, time_e - time_s);
	}

	for(int i=0;i<ALLOC_FREE_PARAM_NUM;i++){

		time_s = ksceKernelGetSystemTimeWide();

		ksceKernelFreeHeapMemory(0x1000B, pResult[i]);

		time_e = ksceKernelGetSystemTimeWide();

		ksceDebugPrintf("%02d %p %10lld usec\n", i, pResult[i], time_e - time_s);
		pResult[i] = NULL;
	}

	for(int i=0;i<ALLOC_FREE_PARAM_NUM;i++){

		SceAllocOpt opt;
		memset(&opt, 0, sizeof(opt));

		opt.size  = sizeof(opt);
		opt.align = allocFreeTestParam[i].align;

		time_s = ksceKernelGetSystemTimeWide();

		pResult[i] = ksceKernelAllocHeapMemoryWithOption(0x1000B, allocFreeTestParam[i].length, &opt);

		time_e = ksceKernelGetSystemTimeWide();

		ksceDebugPrintf("%02d %p align=0x%08X length=0x%08X %10lld usec\n", i, pResult[i], allocFreeTestParam[i].align, allocFreeTestParam[i].length, time_e - time_s);
	}

	for(int i=0;i<ALLOC_FREE_PARAM_NUM;i++){

		time_s = ksceKernelGetSystemTimeWide();

		ksceKernelFreeHeapMemory(0x1000B, pResult[i]);

		time_e = ksceKernelGetSystemTimeWide();

		ksceDebugPrintf("%02d %p %10lld usec\n", i, pResult[i], time_e - time_s);
		pResult[i] = NULL;
	}

	return 0;
}

int alloc_free_test_netps(void){

	SceInt64 time_s, time_e;

	ksceDebugPrintf("%s\n", __FUNCTION__);

	for(int i=0;i<ALLOC_FREE_PARAM_NUM;i++){

		time_s = ksceKernelGetSystemTimeWide();

		pResult[i] = sceNetPsMalloc(allocFreeTestParam[i].length, SCE_NETPS_MALLOC_FLAG_NOT_SLEEP, 8);

		time_e = ksceKernelGetSystemTimeWide();

		ksceDebugPrintf("%02d %p length=0x%08X %10lld usec\n", i, pResult[i], allocFreeTestParam[i].length, time_e - time_s);
	}

	for(int i=0;i<ALLOC_FREE_PARAM_NUM;i++){

		time_s = ksceKernelGetSystemTimeWide();

		sceNetPsFree(pResult[i]);

		time_e = ksceKernelGetSystemTimeWide();

		ksceDebugPrintf("%02d %p %10lld usec\n", i, pResult[i], time_e - time_s);
		pResult[i] = NULL;
	}

	for(int i=0;i<ALLOC_FREE_PARAM_NUM;i++){

		time_s = ksceKernelGetSystemTimeWide();

		pResult[i] = sceNetPsMalloc(allocFreeTestParam[i].length, SCE_NETPS_MALLOC_FLAG_NOT_SLEEP, allocFreeTestParam[i].align);

		time_e = ksceKernelGetSystemTimeWide();

		ksceDebugPrintf("%02d %p align=0x%08X length=0x%08X %10lld usec\n", i, pResult[i], allocFreeTestParam[i].align, allocFreeTestParam[i].length, time_e - time_s);
	}

	for(int i=0;i<ALLOC_FREE_PARAM_NUM;i++){

		time_s = ksceKernelGetSystemTimeWide();

		sceNetPsFree(pResult[i]);

		time_e = ksceKernelGetSystemTimeWide();

		ksceDebugPrintf("%02d %p %10lld usec\n", i, pResult[i], time_e - time_s);
		pResult[i] = NULL;
	}

	return 0;
}

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp){

	int res;

	res = get_netps_function();
	if(res < 0)
		return SCE_KERNEL_START_FAILED;

	res = get_sysmem_function();
	if(res < 0)
		return SCE_KERNEL_START_FAILED;

	alloc_free_test_system_internal();
	alloc_free_test_system_external();
	alloc_free_test_netps();

	return SCE_KERNEL_START_NO_RESIDENT;
}
