/*
 * SceVfs mount PoC
 * Copyright (C) 2020 Princess of Sleeping
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include <psp2kern/kernel/sysmem.h>
#include <string.h>
#include "sce_cpu.h"

void *my_memcpy(void *dst, const void *src, SceSize len);
void sceKernelCpuDcacheWritebackInvalidateRange(void *ptr, SceSize size);
void sceKernelCpuIcacheAndL2WritebackInvalidateRange(void *ptr, SceSize size);
void sceKernelCpuDcacheWritebackRange(void *ptr, SceSize size);

void *SceL2CacheRegBase;

int init_l2_cache_reg(void){

	SceKernelAllocMemBlockKernelOpt opt;

	memset(&opt, 0, sizeof(opt));
	opt.size  = sizeof(opt);
	opt.paddr = (0xD001 << 0xD);
	opt.attr  = SCE_KERNEL_ALLOC_MEMBLOCK_ATTR_HAS_PADDR;

	SceUID memid = ksceKernelAllocMemBlock("SceL2CacheRegVfs", 0x20100206, 0x1000, &opt);

	ksceKernelGetMemBlockBase(memid, &SceL2CacheRegBase);

	return 0;
}

void cache_flush(SceUID pid, uintptr_t addr, SceSize size){

	unsigned int non_align = addr & 0x1F;
	if(non_align != 0)
		size += non_align;

	addr = addr & ~0x1F;
	size = (size + 0x1F) & ~0x1F;

	sceKernelCpuDcacheWritebackInvalidateRange((void *)addr, size);
	sceKernelCpuIcacheAndL2WritebackInvalidateRange((void *)addr, size);

	asm volatile ("isb" ::: "memory");
}

int sceKernelCpuUnrestrictedMemcpy(void *dst, const void *src, SceSize len){

	int prev_dacr;

	asm volatile("mrc p15, 0, %0, c3, c0, 0" : "=r" (prev_dacr));
	asm volatile("mcr p15, 0, %0, c3, c0, 0" :: "r" (0xFFFF0000));

	my_memcpy(dst, src, len);

	unsigned int non_align = (unsigned int)(((uintptr_t)dst) & 0x1F);
	if(non_align != 0)
		len += non_align;

	dst = (void *)(((uintptr_t)dst) & ~0x1F);
	len = (len + 0x1F) & ~0x1F;

	sceKernelCpuDcacheWritebackRange(dst, len);

	asm volatile("mcr p15, 0, %0, c3, c0, 0" :: "r" (prev_dacr));
	return 0;
}

int memcpy_rx(SceUID pid, void *dst, const void *src, SceSize len){

	sceKernelCpuUnrestrictedMemcpy(dst, src, len);
	cache_flush(pid, (uintptr_t)dst, len);
	return 0;
}
