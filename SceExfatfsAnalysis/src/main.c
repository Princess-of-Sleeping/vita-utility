/*
 * SceExfatfs Analysis
 * Copyright (C) 2021 Princess of Sleeping
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/sysmem.h>
#include <psp2kern/kernel/sysclib.h>
#include <psp2kern/io/fcntl.h>
#include "log.h"

int write_file(const char *path, const void *data, SceSize size){

	if(data == NULL || size == 0)
		return -1;

	SceUID fd = ksceIoOpen(path, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0666);
	if (fd < 0)
		return fd;

	ksceIoWrite(fd, data, size);
	ksceIoClose(fd);

	return 0;
}

typedef struct SceExfatfsFileCache SceExfatfsFileCache;

typedef struct exfat_ctx { //size is 0x430 bytes

	const char *blk_name;
	uint32_t unk_4;
	uint32_t unk_8;
	uint32_t rsvd_sector;

	uint32_t unk_10;
	uint32_t available_clusters; // Number of clusters available
	SceExfatfsFileCache* unk_18;
	uint32_t unk_1C;

	uint32_t unk_20;
	uint32_t unk_24;
	uint32_t unk_28;
	uint32_t unk_2C;

	uint32_t unk_30;
	uint32_t unk_34;
	uint32_t unk_38;
	uint32_t unk_3C;

	uint32_t unk_40;
	uint32_t unk_44;
	uint32_t unk_48;
	uint32_t unk_4C;

	uint32_t unk_50;
	uint32_t unk_54;
	uint32_t unk_58;
	uint32_t unk_5C;

	uint32_t unk_60;
	uint32_t unk_64;
	uint32_t unk_68;
	uint32_t unk_6C;

	uint32_t unk_70;
	uint32_t unk_74;
	uint32_t unk_78;
	uint32_t unk_7C;

	uint32_t unk_80;
	uint32_t unk_84;
	uint32_t unk_88;
	uint32_t unk_8C;

	uint32_t unk_90;
	uint32_t unk_94;
	uint32_t unk_98;
	uint32_t unk_9C;

	uint32_t unk_A0;
	uint32_t unk_A4;
	uint32_t unk_A8;
	uint32_t unk_AC;

	uint32_t unk_B0;
	uint32_t unk_B4; // maybe fs type
	uint32_t unk_B8;
	uint32_t unk_BC;

	uint32_t unk_C0;
	uint32_t unk_C4;
	uint32_t blk_size; // sector num
	uint32_t unk_CC;

	char data1[0x310];
	uint32_t fast_mutex_SceExfatfsRoot; //offset 0x3E0, 7B0C90
	char data2[0x4C];
} SceExfatfsPartCtx;

typedef struct SceExfatfsDateTime { // size is 0xC
	unsigned short year;
	unsigned short month;
	unsigned short day;
	unsigned short hour;
	unsigned short minute;
	unsigned short second;
} SceExfatfsDateTime;

typedef struct SceExfatfsFileCache { // size is 0x290 bytes
	uint16_t path[0x208 >> 1];   // in unicode
	uint16_t *pFileName;
	SceExfatfsPartCtx *data_0x20C;
	SceMode mode;
	unsigned int attr;
	SceOff size;
	SceExfatfsDateTime ctime;
	SceExfatfsDateTime atime;
	SceExfatfsDateTime mtime;
	uint32_t cluster_start;
	uint32_t data_0x248;
	uint32_t data_0x24C;
	uint32_t data_0x250;
	uint32_t data_0x254;
	uint32_t data_0x258;
	uint32_t data_0x25C;
	uint32_t data_0x260;
	uint32_t data_0x264;
	uint32_t data_0x268;
	uint32_t data_0x26C;
	uint32_t data_0x270;
	uint32_t data_0x274;
	uint32_t data_0x278;
	void *data_0x27C;
	uint32_t data_0x280; // cluster_end
	uint32_t data_0x284; // (cluster_num - 1) or 0
	uint32_t data_0x288; // cluster_num
	uint32_t data_0x28C;
} SceExfatfsFileCache;

int WCharToChar(char *dst, uint16_t *src, int len){

	int idx = 0;

	while(idx < len && src[idx] != 0){
		*dst++ = (char)src[idx];
		if(src[idx] >= 0x100)
			*dst++ = (char)(src[idx] >> 8);
		idx++;
	}

	return 0;
}

/*
 * TODO
 * look SceExfatfs_func_0x81008198
 */

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp){

	SceUID moduleid = ksceKernelSearchModuleByName("SceExfatfs");

	SceKernelModuleInfo sce_info;
	memset(&sce_info, 0, sizeof(sce_info));
	sce_info.size = sizeof(sce_info);
	ksceKernelGetModuleInfo(0x10005, moduleid, &sce_info);

	SceExfatfsPartCtx *pPartCtxList = (SceExfatfsPartCtx *)((uintptr_t)sce_info.segments[1].vaddr + 0x80);

	/*
	 * maybe
	 * { // SceExfatfs_data + 0x5D50
	 * 	void *data0;
	 * 	int data1;
	 * 	SceExfatfsFileCache list[];
	 * }
	 */
	SceExfatfsFileCache *pCacheList = (SceExfatfsFileCache *)((uintptr_t)sce_info.segments[1].vaddr + 0x5D58);

	ksceIoRemove("sd0:dump/sce_exfatfs.txt");
	LogOpen("sd0:dump/sce_exfatfs.txt");

	LogWrite("text %p, 0x%X\n", sce_info.segments[0].vaddr, sce_info.segments[0].memsz);
	LogWrite("data %p, 0x%X\n", sce_info.segments[1].vaddr, sce_info.segments[1].memsz);
	LogWrite("\n");

	for(int i=0;i<10;i++){
		LogWrite("0x%08X\n", i);
		LogWrite("%s\n", pPartCtxList[i].blk_name);
		LogWrite("unk_4             :0x%08X\n", pPartCtxList[i].unk_4);
		LogWrite("unk_8             :0x%08X\n", pPartCtxList[i].unk_8);
		LogWrite("rsvd_sector       :0x%08X\n", pPartCtxList[i].rsvd_sector);
		LogWrite("unk_10            :0x%08X\n", pPartCtxList[i].unk_10);
		LogWrite("available_clusters:0x%08X\n", pPartCtxList[i].available_clusters);
		LogWrite("unk_1C            :0x%08X\n", pPartCtxList[i].unk_1C);

		if(pPartCtxList[i].unk_B4 == 1){
			LogWrite("FAT16\n");
		}else if(pPartCtxList[i].unk_B4 == 2){
			LogWrite("exFAT\n");
		}else{
			LogWrite("unk_B4:0x%08X\n", pPartCtxList[i].unk_B4);
		}
		LogWrite("blk_size:0x%08X(0x%08X%08X byte)\n", pPartCtxList[i].blk_size, pPartCtxList[i].blk_size >> 23, pPartCtxList[i].blk_size << 9);
		LogWrite("\n");
	}

	for(int i=0;i<445;i++){
		if(pCacheList[i].path[0] != 0){
			LogWrite("%03d\n", i);

			if(pCacheList[i].path[0] == '/'){
				char path[0x208];
				memset(path, 0, sizeof(path));
				WCharToChar(path, pCacheList[i].path, 0x104);
				LogWrite("path:%s\n", path);
			}

			LogWrite("0x%llX byte\n", pCacheList[i].size);
			LogWrite("blk_name:%s\n", pCacheList[i].data_0x20C->blk_name);

			LogWrite(
				"ctime:%04d/%02d/%02d %02d/%02d/%02d\n",
				pCacheList[i].ctime.year,
				pCacheList[i].ctime.month,
				pCacheList[i].ctime.day,
				pCacheList[i].ctime.hour,
				pCacheList[i].ctime.minute,
				pCacheList[i].ctime.second
			);

			LogWrite(
				"atime:%04d/%02d/%02d %02d/%02d/%02d\n",
				pCacheList[i].atime.year,
				pCacheList[i].atime.month,
				pCacheList[i].atime.day,
				pCacheList[i].atime.hour,
				pCacheList[i].atime.minute,
				pCacheList[i].atime.second
			);

			LogWrite(
				"mtime:%04d/%02d/%02d %02d/%02d/%02d\n",
				pCacheList[i].mtime.year,
				pCacheList[i].mtime.month,
				pCacheList[i].mtime.day,
				pCacheList[i].mtime.hour,
				pCacheList[i].mtime.minute,
				pCacheList[i].mtime.second
			);
			LogWrite("cluster_start : 0x%08X\n", pCacheList[i].cluster_start);

			if(pCacheList[i].data_0x20C->unk_B4 == 1){

				if(pCacheList[i].cluster_start == 0){
					LogWrite("entry offset : 0x%08X%08X [B]\n", 0, pCacheList[i].data_0x20C->rsvd_sector << 9);
				}else{
					uint32_t entry_sector = (pCacheList[i].data_0x20C->rsvd_sector + ((pCacheList[i].cluster_start - 2) << 3) + 0x20);
					LogWrite("entry offset : 0x%08X%08X [B]\n", entry_sector >> 23, entry_sector << 9);
				}
			}

			LogWrite("\n");
		}
	}

	LogWrite("\n");
	LogClose();

	if(0){
		void *pSceExfafsData;

		SceUID memid = ksceKernelAllocMemBlock("SceExfafsData", 0x6020D006, 0x400000, NULL);
		ksceKernelGetMemBlockBase(memid, &pSceExfafsData);

		memcpy(pSceExfafsData, sce_info.segments[1].vaddr, sce_info.segments[1].memsz);
		write_file("sd0:dump/SceExfatfs_data.bin", pSceExfafsData, sce_info.segments[1].memsz);

		ksceKernelFreeMemBlock(memid);
	}

	if(0){
		SceIoStat stat;
		ksceIoGetstat("os0:sm/update_service_sm.self", &stat);
		write_file("sd0:dump/stat.bin", &stat, sizeof(stat));
	}

	return SCE_KERNEL_START_SUCCESS;
}
