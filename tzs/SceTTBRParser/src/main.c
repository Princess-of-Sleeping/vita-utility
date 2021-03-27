/*
 * SceTzs TTBR parser(+ play with modulemgr data)
 * Copyright (C) 2021, Princess of Sleeping
 */

#include <psp2/kernel/modulemgr.h>
#include <psp2/kernel/sysmem.h>
#include <psp2/kernel/clib.h>
#include <psp2/io/fcntl.h>
#include "modulemgr_internal.h"

/*
 * If base on Non-secure kernel, set to 0x40200000.
 */
#define SCE_MEMORY_BASE (0x40000000)

int sceAppMgrDestroyAppByAppId(int appid);

void *pTzs;
uintptr_t *pTTBR0, *pTTBR1;

uint8_t mmuRead8(uintptr_t addr){

	uintptr_t *pTTBR;

	if(addr < 0x40000000){
		pTTBR = pTTBR0;
	}else{
		pTTBR = pTTBR1;
	}

	uintptr_t ttbr = pTTBR[(addr >> 20) & 0xFFF];

	// sceClibPrintf("%s:ttbr 0x%X\n", __FUNCTION__, ttbr);

	if((ttbr & 3) == 1){

		uintptr_t second_level_section = ttbr & 0xFFFFFC00;

		uintptr_t *pTTBRSec = pTzs + (second_level_section - SCE_MEMORY_BASE);

		uintptr_t ttbr_sec = pTTBRSec[(addr >> 12) & 0xFF];

		if((ttbr_sec & 3) == 1){ // large
			return *(uint8_t *)(pTzs + ((ttbr_sec & 0xFFFF0000) - SCE_MEMORY_BASE) + (addr & 0xFFFF));
		}else if((ttbr_sec & 2) != 0){ // small
			return *(uint8_t *)(pTzs + ((ttbr_sec & 0xFFFFF000) - SCE_MEMORY_BASE) + (addr & 0xFFF));
		}

	}else if((ttbr & 3) == 2){

		uintptr_t ttbr = pTTBR[(addr >> 20) & 0xFFF];

		if(ttbr == 0x1E0)
			goto error;

		return *(uint8_t *)(pTzs + ((ttbr & 0xFFF00000) - SCE_MEMORY_BASE) + (addr & 0xFFFFF));
	}

error:
	sceClibPrintf("%s:error\n", __FUNCTION__);
	while(1){}
}

uint16_t mmuRead16(uintptr_t addr){

	uint16_t res;

	res  = (mmuRead8(addr + 0) << 0);
	res |= (mmuRead8(addr + 1) << 8);

	return res;
}

uint32_t mmuRead32(void *addr){

	uint32_t res;

	res  = (mmuRead16((uintptr_t)addr + 0) << 0);
	res |= (mmuRead16((uintptr_t)addr + 2) << 16);

	return res;
}

void *mmuRead32_ptr(void *addr){
	return (void *)mmuRead32(addr);
}

char *mmuStrcpy(char *dst, const void *src){

	char s;

	do {
		s = (char)mmuRead8((uintptr_t)src);
		*dst++ = s;
		src = (const void *)((uintptr_t)src + 1);
	} while(s != 0);

	return dst;
}

int parse_ttbr_page_table(int ttbr_idx, uintptr_t TTBR){

	uintptr_t second_level_section = TTBR & 0xFFFFFC00;

	uintptr_t *pTTBRSec = pTzs + (second_level_section - SCE_MEMORY_BASE);

	for(int i=0;i<0x100;i++){
		uintptr_t ttbr_sec = pTTBRSec[i];
		if(ttbr_sec == 0)
			continue;

		if((ttbr_sec & 3) == 1){
			sceClibPrintf("[%-15s] 0x%08X vaddr:0x%08X paddr:0x%08X\n", "page_table", TTBR, (ttbr_idx << 20) | (i << 12), ttbr_sec & 0xFFFF0000);
		}else if((ttbr_sec & 2) != 0){
			sceClibPrintf("[%-15s] 0x%08X vaddr:0x%08X paddr:0x%08X\n", "page_table", TTBR, (ttbr_idx << 20) | (i << 12), ttbr_sec & 0xFFFFF000);
		}
	}

	return 0;
}

int parse_ttbr_section(int ttbr_idx, uintptr_t TTBR){

	sceClibPrintf("[%-15s] 0x%08X vaddr:0x%08X paddr:0x%08X\n", "section", TTBR, ttbr_idx << 20, TTBR & 0xFFF00000);

	return 0;
}

int parse_ttbr(uintptr_t *pTTBR){

	for(int i=0;i<0x1000;i++){
		uintptr_t ttbr = pTTBR[i];
		if(ttbr == 0x1E0)
			continue;

		if((ttbr & 3) == 0){
			sceClibPrintf("Fault\n");
		}else if((ttbr & 3) == 1){
			parse_ttbr_page_table(i, ttbr);
		}else if((ttbr & 3) == 2){
			parse_ttbr_section(i, ttbr);
		}else if((ttbr & 3) == 3){
			sceClibPrintf("Super section\n");
		}
	}

	return 0;
}

void print_module_load_info(SceModuleInfoInternal *pModuleInfo){

	char *name = mmuRead32_ptr(&pModuleInfo->module_name);

	char module_name[0x20];

	mmuStrcpy(module_name, name);

	if(1){
		if(mmuRead32(&pModuleInfo->segments_num) < 2){
			sceClibPrintf("[%-27s]:text=0x%08x(0x%08x), (no data)\n",
				module_name, mmuRead32(&pModuleInfo->segments[0].vaddr), mmuRead32(&pModuleInfo->segments[0].memsz)
			);
		}else{
			sceClibPrintf("[%-27s]:text=0x%08x(0x%08x), data=0x%08x(0x%08x/0x%08x)\n",
				module_name,
				mmuRead32(&pModuleInfo->segments[0].vaddr), mmuRead32(&pModuleInfo->segments[0].memsz),
				mmuRead32(&pModuleInfo->segments[1].vaddr), mmuRead32(&pModuleInfo->segments[1].filesz), mmuRead32(&pModuleInfo->segments[1].memsz)
			);
		}
	}
}

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp){

	SceUID memid;

	memid = sceKernelAllocMemBlock("SceTzs", 0x0C20D060, 0x200000, NULL);
	sceKernelGetMemBlockBase(memid, (void **)&pTzs);

	memid = sceKernelAllocMemBlock("SceTzsTTBR0", 0x0C20D060, 0x4000, NULL);
	sceKernelGetMemBlockBase(memid, (void **)&pTTBR0);

	memid = sceKernelAllocMemBlock("SceTzsTTBR1", 0x0C20D060, 0x4000, NULL);
	sceKernelGetMemBlockBase(memid, (void **)&pTTBR1);

	SceUID fd;

	fd = sceIoOpen("host0:tzs.bin", 1, 0);
	if(fd < 0)
		fd = sceIoOpen("sd0:tzs.bin", 1, 0);
	if(fd < 0)
		fd = sceIoOpen("uma0:tzs.bin", 1, 0);
	if(fd < 0)
		fd = sceIoOpen("ux0:tzs.bin", 1, 0);
	if(fd < 0){
		sceAppMgrDestroyAppByAppId(~2);
		while(1){
			sceKernelDelayThread(10000);
		}
	}

	sceIoLseek(fd, 0LL, SCE_SEEK_SET);
	sceIoRead(fd, pTzs, 0x200000);

	sceIoLseek(fd, 0x108000LL, SCE_SEEK_SET);
	sceIoRead(fd, pTTBR0, 0x4000);

	sceIoLseek(fd, 0x10C000LL, SCE_SEEK_SET);
	sceIoRead(fd, pTTBR1, 0x4000);

	sceIoClose(fd);

	if(0){
		parse_ttbr(pTTBR0);
		parse_ttbr(pTTBR1);
	}

	uintptr_t process_module_info_address = 0x400737A4; // 3.60
	SceKernelProcessModuleInfo *pProcModuleInfo = *(SceKernelProcessModuleInfo **)(pTzs + (process_module_info_address - SCE_MEMORY_BASE));

	SceUID pid = mmuRead32(&pProcModuleInfo->pid);
	sceClibPrintf("pid:0x%X\n", pid);

	SceModuleInfoInternal *pModuleInfo = mmuRead32_ptr(&pProcModuleInfo->pModuleInfo);

	while(pModuleInfo != NULL){
		print_module_load_info(pModuleInfo);
		pModuleInfo = mmuRead32_ptr(&pModuleInfo->next);
	}

	sceAppMgrDestroyAppByAppId(~2);

	return SCE_KERNEL_START_SUCCESS;
}
