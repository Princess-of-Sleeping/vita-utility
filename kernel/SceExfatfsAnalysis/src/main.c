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
#include <psp2kern/kernel/debug.h>
#include <psp2kern/io/fcntl.h>
#include <taihen.h>
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
	int cluster_start;
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
	uint32_t data_0x270; // this is uint16_t
	uint32_t data_0x274;
	uint32_t data_0x278;
	void *data_0x27C;
	uint32_t data_0x280; // cluster_end
	uint32_t data_0x284; // (cluster_num - 1) or 0
	uint32_t data_0x288; // cluster_num
	uint32_t data_0x28C;
} SceExfatfsFileCache;

int WCharToChar(char *dst, const uint16_t *src, int len){

	int idx = 0;

	while(idx < len && src[idx] != 0){
		*dst++ = (char)src[idx];
		if(src[idx] >= 0x100)
			*dst++ = (char)(src[idx] >> 8);
		idx++;
	}

	return 0;
}

// create cache

/*
 * pData - SceExfatfs_data + 0x80
 */
tai_hook_ref_t sub_81006BB0_ref;
SceExfatfsFileCache *sub_81006BB0_patch(void *pData, SceExfatfsPartCtx *pCtx, uint16_t *path){

	const void *lr;
	asm volatile("mov %0, lr\n":"=r"(lr));

	SceExfatfsFileCache *res = TAI_CONTINUE(SceExfatfsFileCache *, sub_81006BB0_ref, pData, pCtx, path);


	char loc_path[0x204];
	memset(loc_path, 0, sizeof(loc_path));

	WCharToChar(loc_path, path, 0x104);

	ksceDebugPrintf("sub_81006BB0:lr(0x%X):0x%X %-27s:%s\n", lr, res, pCtx->blk_name, loc_path);


	// don't use uma0 cache
	if(res != NULL && strcmp(pCtx->blk_name, "sdstor0:uma-pp-act-a") == 0){

		memset(res, 0, sizeof(*res));

		res->pFileName = res->path;
		res->cluster_start = -1;
		res->data_0x250 = ~1;
		res->data_0x260 = ~1;
		res->data_0x280 = ~1;
		res->data_0x284 = ~1;

		res = NULL;
	}

	return res;
}

tai_hook_ref_t sub_81003384_ref;
int sub_81003384_patch(SceExfatfsFileCache *info, const char *name, SceSize name_length, void *a4){

	const void *lr;
	asm volatile("mov %0, lr\n":"=r"(lr));

	char loc_path[0x204];
	memset(loc_path, 0, sizeof(loc_path));

	WCharToChar(loc_path, info->path, 0x104);

	int res = TAI_CONTINUE(int, sub_81003384_ref, info, name, name_length, a4);

	ksceDebugPrintf("sub_81003348:lr(0x%X):0x%X %s %s:%s:0x%08X:0x%08X\n", lr, res, info->data_0x20C->blk_name, loc_path, name, name_length, a4);

	return res;
}

tai_hook_ref_t sub_81009CA0_ref;
int sub_81009CA0_patch(int a1){

	const void *lr;
	asm volatile("mov %0, lr\n":"=r"(lr));

	int res = TAI_CONTINUE(int, sub_81009CA0_ref, a1);

	ksceDebugPrintf("sub_81009CA0:lr(0x%X):0x%X 0x%08X\n", lr, res, *(int *)a1);

	return res;
}

/*
 * TODO
 * look SceExfatfs_func_0x81008198
 */

#define HookOffset(modid, offset, thumb, func_name) \
	taiHookFunctionOffsetForKernel(0x10005, &func_name ## _ref, modid, 0, offset, thumb, func_name ## _patch)

#define HookExport(module_name, library_nid, func_nid, func_name) \
	taiHookFunctionExportForKernel(0x10005, &func_name ## _ref, module_name, library_nid, func_nid, func_name ## _patch)

typedef struct SceVfsAdd {     // size is 0x20
	const void *func_ptr1;
	const char *device;    // ex:"bsod_dummy_host_fs"
	int data_0x08;         // ex:0x11
	int data_0x0C;
	int data_0x10;         // ex:0x10
	const void *func_ptr2;
	int data_0x18;
	struct SceVfsAdd *prev;
} SceVfsAdd;

const char patch1[] = {
	0x00, 0x20, 0x02, 0x90,
	0x00, 0xBF, 0x00, 0xBF,
	0x00, 0xBF, 0x00, 0xBF
};

const char patch2[] = {
	0x00, 0x20, 0x00, 0xBF,
	0x00, 0xBF, 0x00, 0xBF
};

const char patch3[] = {
	0x00, 0x20, 0x00, 0xBF,
	0x00, 0xBF, 0x00, 0xBF,
	0x00, 0xBF, 0x00, 0xBF,
	0x00, 0xBF
};

typedef struct SceUIDIoMountEventClass { // size is 0x4C
/*
	int data_0x00;
	void *data_0x04;
*/
	uint32_t sce_rsvd[2];
	int data_0x08;
	void *data_0x0C;
	void *data_0x10;
	void *data_0x14;
	int data_0x18;
	int data_0x1C;    // 1
	int data_0x20;    // 0x100
	int data_0x24;
	void *data_0x28;
	void *data_0x2C;

	int data_0x30;
	int data_0x34;
	SceUID data_0x38; // this obj uid
	int data_0x3C;    // 0x202

	int data_0x40;
	void *data_0x44;
	struct SceUIDIoMountEventClass *next;
} SceUIDIoMountEventClass;

typedef struct SceIoPartConfig {
	const char *device;
	const char *blockdev_fs;
	const char *device_block[2];
	int mount_id;
} SceIoPartConfig;

typedef struct SceIoPartEntry { // size is 0x38
	int mount_id;
	const char *dev_unix;
	int data_0x0C;
	int16_t dev_major[2];

	int8_t dev_minor[4];
	const char *dev_fs;
	struct {
		int unk;
		SceIoPartConfig *config;
	} ent[2];

	SceUIDIoMountEventClass *mount_event;
	int data_0x2C;
	int data_0x30;
	int data_0x34;
} SceIoPartEntry;

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp){

	SceKernelModuleInfo sce_info;
	SceUID moduleid = ksceKernelSearchModuleByName("SceExfatfs");

	// HookOffset(moduleid, 0x6BB0, 1, sub_81006BB0); // create cache
	// HookOffset(moduleid, 0x3384, 1, sub_81003384);
	// HookOffset(moduleid, 0x9CA0, 1, sub_81009CA0);

	// disable cache test, but if enable this, vita non bootable from next open file time.
	if(0){
		// part init
		taiInjectDataForKernel(0x10005, moduleid, 0, 0x9D1A, patch1, sizeof(patch1));
		taiInjectDataForKernel(0x10005, moduleid, 0, 0x9D2C, patch2, sizeof(patch2));

		// vfs_ioopen
		char _patch = 0xE0;
		taiInjectDataForKernel(0x10005, moduleid, 0, 0x9226, patch3, sizeof(patch3));
		taiInjectDataForKernel(0x10005, moduleid, 0, 0x923D, &_patch, 1);

		// vfs_getiostat
		taiInjectDataForKernel(0x10005, moduleid, 0, 0x9F06, patch2, sizeof(patch2));
	}

	// SceIofilemgr test
	if(0){
		moduleid = ksceKernelSearchModuleByName("SceIofilemgr");

		memset(&sce_info, 0, sizeof(sce_info));
		sce_info.size = sizeof(sce_info);
		ksceKernelGetModuleInfo(0x10005, moduleid, &sce_info);

		// SceVfs tree test
		if(0){
			SceVfsAdd *pSceVfsAdd = *(SceVfsAdd **)((uintptr_t)sce_info.segments[1].vaddr + 0x19F4);

			while(pSceVfsAdd != NULL){
				ksceDebugPrintf("%s\n", pSceVfsAdd->device);
				pSceVfsAdd = pSceVfsAdd->prev;
			}
		}

		// SceIoPartEntry array test
		if(0){
			SceIoPartEntry *pSceIoPartEntry = (SceIoPartEntry *)((uintptr_t)sce_info.segments[1].vaddr + 0x1A90);

			for(int i=0;i<0x20;i++){
				if(pSceIoPartEntry[i].mount_id == 0)
					continue;

				ksceDebugPrintf("%s\n", pSceIoPartEntry[i].ent[0].config->device);
			}
		}
	}

	return 0;

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
					// TODO:fix this
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
