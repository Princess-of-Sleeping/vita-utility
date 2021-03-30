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
int (* mapping_vaddr_by_paddr)(void *ttbr, int memtype, int domain, const void *vaddr, SceSize size, unsigned int paddr) = NULL;

int ksceKernelCpuDcacheCleanInvalidateMVAC(void *a1);

int module_get_offset(SceUID pid, SceUID modid, int segidx, uint32_t offset, uintptr_t *dst);

unsigned int *pTTBR0, *pTTBR1;

int map_vaddr(int memtype, int domain, uintptr_t vaddr, SceSize size, uintptr_t paddr){

	unsigned int *pTTBR;

	if(vaddr < 0x40000000){
		pTTBR = pTTBR0;
	}else{
		pTTBR = pTTBR1;
	}

	mapping_vaddr_by_paddr(pTTBR, memtype, domain, (const void *)vaddr, size, paddr);

	ksceKernelCpuDcacheCleanInvalidateMVAC(&pTTBR[(vaddr >> 20) & 0xFFF]);

	return 0;
}

void _start() __attribute__ ((weak, alias ("module_start")));
int module_start(SceSize argc, const void *args){

	tai_module_info_t tai_info;
	tai_info.size = sizeof(tai_module_info_t);

	if(taiGetModuleInfoForKernel(0x10005, "SceSysmem", &tai_info) < 0){
		ksceDebugPrintf("taiGetModuleInfoForKernel failed\n");
		return SCE_KERNEL_START_SUCCESS;
	}

	module_get_offset(0x10005, tai_info.modid, 0, 0x2364C | 1, (uintptr_t *)&mapping_vaddr_by_paddr);

	unsigned int ttbr0, ttbr1;

	__asm__ volatile(
		"mrc p15, #0, %0, c2, c0, #0\n"
		"mrc p15, #0, %1, c2, c0, #1\n"
		: "=r"(ttbr0), "=r"(ttbr1)
	);

	ttbr0 &= ~0xFFF;
	ttbr1 &= ~0xFFF;

	pTTBR0 = pa2va(ttbr0);
	pTTBR1 = pa2va(ttbr1);

	ksceDebugPrintf("ttbr0 : 0x%X\n", ttbr0);
	ksceDebugPrintf("ttbr1 : 0x%X\n", ttbr1);

	map_vaddr(0x10200206, 0xC, 0x51000000, 0x100000, 0x51000000);

	SceUID fd;

	fd = ksceIoOpen("host0:nskbl_dump.bin", SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0666);
	if(fd < 0)
		fd = ksceIoOpen("sd0:nskbl_dump.bin", SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0666);
	if(fd < 0)
		fd = ksceIoOpen("ux0:nskbl_dump.bin", SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0666);
	if(fd >= 0){
		ksceIoWrite(fd, (void *)0x51000000, 0x100000);
		ksceIoClose(fd);
	}

	return SCE_KERNEL_START_NO_RESIDENT;
}
