/*
 * kernel custom malloc
 * Copyright (C) 2020 Princess of Sleeping
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/sysmem.h>
#include <taihen.h>
#include <string.h>

int module_get_offset(SceUID pid, SceUID modid, int segidx, uint32_t offset, uintptr_t *dst);

int module_get_export_func(SceUID pid, const char *modname, uint32_t lib_nid, uint32_t func_nid, uintptr_t *func);
#define GetExport(modname, lib_nid, func_nid, func) module_get_export_func(KERNEL_PID, modname, lib_nid, func_nid, (uintptr_t *)func)

int (* init_malloc_internal_ctx)(void *a1, void *a2, int a3, void *a4, int a5);
int (* init_malloc_ctx)(void *a1, int a2, void *ctx, int a4);

void *(* SceSysmemForKernel_4D38F861)(void);
void *(* sceUIDtoObject)(SceUID uid);

char init_buff1[0xC4];
char init_buff2[0x48];

void *(* sceKernelAllocInternal)(void *ctx, SceSize len, SceAllocOpt *pOpt);
void (* sceKernelFreeInternal)(void *ctx, void *ptr);

int malloc_test(void *ctx, SceSize len, SceSize align){

	SceAllocOpt opt;
	memset(&opt, 0, sizeof(opt));

	opt.size = sizeof(opt);
	opt.align = align;

	void *ptr = sceKernelAllocInternal(ctx, len, &opt);
	if(ptr != NULL){
		ksceDebugPrintf("sceKernelAllocInternal 0x%X(align:0x%X)\n", ptr, align);
		sceKernelFreeInternal(ctx, ptr);
		ptr = NULL;
	}else{
		ksceDebugPrintf("sceKernelAllocInternal failed\n");
	}

	return 0;
}

void _start() __attribute__ ((weak, alias ("module_start")));
int module_start(SceSize argc, const void *args){

	tai_module_info_t tai_info;
	tai_info.size = sizeof(tai_module_info_t);

	if(taiGetModuleInfoForKernel(KERNEL_PID, "SceSysmem", &tai_info) < 0){
		ksceDebugPrintf("taiGetModuleInfoForKernel failed\n");
		goto start_failed;
	}

	module_get_offset(KERNEL_PID, tai_info.modid, 0, 0xD20 | 1, (uintptr_t *)&init_malloc_internal_ctx);
	module_get_offset(KERNEL_PID, tai_info.modid, 0, 0xD7C | 1, (uintptr_t *)&init_malloc_ctx);
	module_get_offset(KERNEL_PID, tai_info.modid, 0, 0x1557C | 1, (uintptr_t *)&sceKernelAllocInternal);
	module_get_offset(KERNEL_PID, tai_info.modid, 0, 0x15608 | 1, (uintptr_t *)&sceKernelFreeInternal);

	if(GetExport("SceSysmem", 0x63A519E5, 0x4D38F861, &SceSysmemForKernel_4D38F861) < 0)
		goto start_failed;

	if(GetExport("SceSysmem", 0x63A519E5, 0xED221825, &sceUIDtoObject) < 0)
		goto start_failed;

	void *ctx, *sysmem_ptr;

	SceUID heap_uid = ksceKernelCreateHeap("SceKernelHeapForTest", 0x2000, NULL);
	if(heap_uid < 0){
		ksceDebugPrintf("sceKernelCreateHeap failed : 0x%X\n", heap_uid);
		goto start_failed;
	}

	ctx = sceUIDtoObject(heap_uid);

	sysmem_ptr = SceSysmemForKernel_4D38F861();

	init_malloc_internal_ctx(init_buff1, sysmem_ptr, 6, init_buff2, 0);

	init_malloc_ctx(init_buff1, -1, ctx, -1);

	malloc_test(ctx, 0x100, 0x10);
	malloc_test(ctx, 0x100, 0x20);
	malloc_test(ctx, 0x100, 0x40);
	malloc_test(ctx, 0x100, 0x80);
	malloc_test(ctx, 0x100, 0x100);
	malloc_test(ctx, 0x100, 0x200);
	malloc_test(ctx, 0x100, 0x400);
	malloc_test(ctx, 0x100, 0x800);

	return SCE_KERNEL_START_SUCCESS;

start_failed:
	return SCE_KERNEL_START_FAILED;
}

int module_stop(SceSize argc, const void *args){
	return SCE_KERNEL_STOP_SUCCESS;
}
