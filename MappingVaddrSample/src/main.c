/*
 * mapping vaddr sample
 * Copyright (C) 2020 Princess of Sleeping
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/sysmem.h>
#include <psp2kern/io/fcntl.h>
#include <taihen.h>

void *pa2va(unsigned int pa){
	unsigned int va = 0, vaddr, paddr, i;

	for (i = 0; i < 0x100000; i++) {
		vaddr = i << 12;

		__asm__ volatile(
			"mcr p15, #0, %1, c7, c8, #0\n"
			"mrc p15, #0, %0, c7, c4, #0\n"
			: "=r"(paddr)
			: "r"(vaddr)
		);

		if((pa & ~0xFFF) == (paddr & ~0xFFF)){
			va = vaddr + (pa & 0xFFF);
			break;
		}
	}
	return (void *)va;
}

/*
 * mapping_paddr_by_vaddr
 *
 * size must be a multiple of 0x100000
 */
int (* mapping_vaddr_by_paddr)(void *ttbr0, int memtype, int domain, const void *vaddr, SceSize size, unsigned int paddr) = NULL;

int ksceKernelCpuDcacheCleanInvalidateMVAC(void *a1);

int module_get_offset(SceUID pid, SceUID modid, int segidx, uint32_t offset, uintptr_t *dst);

void _start() __attribute__ ((weak, alias ("module_start")));
int module_start(SceSize argc, const void *args){

	tai_module_info_t tai_info;
	tai_info.size = sizeof(tai_module_info_t);

	if(taiGetModuleInfoForKernel(KERNEL_PID, "SceSysmem", &tai_info) < 0){
		ksceDebugPrintf("taiGetModuleInfoForKernel failed\n");
		return SCE_KERNEL_START_SUCCESS;
	}

	module_get_offset(KERNEL_PID, tai_info.modid, 0, 0x2364C | 1, (uintptr_t *)&mapping_vaddr_by_paddr);

	unsigned int ttbr0;
	unsigned int *tbl = NULL;

	__asm__ volatile(
		"mrc p15, #0, %0, c2, c0, #0\n"
		: "=r"(ttbr0)
	);

	ttbr0 &= ~0xFFF;

	tbl = pa2va(ttbr0);

	ksceDebugPrintf("ttbr0 : 0x%X(0x%X)\n", ttbr0, tbl); // ttbr0 : 0x40208000(0x78000)

	mapping_vaddr_by_paddr(tbl, 0x10200206, 0xC, (const void *)0x3F000000, 0x100000, 0x51000000);

	ksceKernelCpuDcacheCleanInvalidateMVAC(&tbl[0x3F0]);

	SceUID fd = ksceIoOpen("sd0:nskbl_dump.bin", SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0666);
	if(fd > 0){
		ksceIoWrite(fd, (void *)0x3F000000, 0x100000);
		ksceIoClose(fd);
	}

	return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args){
	return SCE_KERNEL_STOP_SUCCESS;
}
