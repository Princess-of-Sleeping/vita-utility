/*
 * SceSdstor Analysis
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

typedef struct SceSdstorMbr { // size is 0x200
	char data[0x200];
} SceSdstorMbr;

typedef struct ScePartitionEntrie { // size is 0x11
	uint32_t start_lba;
	uint32_t n_sectors;
	uint8_t part_id;
	uint8_t part_type;
	uint8_t part_flag;
	uint16_t flag;
	uint32_t unused;
} __attribute__((packed)) ScePartitionEntrie;

typedef struct SceSdstorPartInfo { // size is 0x1C
	void *data_0x00; // pointer to SceSdstor_data + 0xA40
	ScePartitionEntrie *data_0x04;
	char id[4];
	int flags;
	int unk_0x10;
	uint32_t start_lba;
	uint32_t n_sectors;
} SceSdstorPartInfo;

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp){

	SceKernelModuleInfo sce_info;
	SceUID moduleid = ksceKernelSearchModuleByName("SceSdstor");


	memset(&sce_info, 0, sizeof(sce_info));
	sce_info.size = sizeof(sce_info);
	ksceKernelGetModuleInfo(0x10005, moduleid, &sce_info);

	SceSdstorMbr *pMbr = (SceSdstorMbr *)((uintptr_t)sce_info.segments[1].vaddr + 0x40);

	char path[0x80];

	for(int i=0;i<5;i++){
		snprintf(path, sizeof(path), "uma0:sdstor_mbr_%d.bin", i);
		write_file(path, &pMbr[i], 0x200);
	}


	/*
	 * SceSdstor_data + 0xA40 is same format to SceExfatfs_data + 0x295DE8.
	 * size is 0x40 byte.
	 */

	ksceIoRemove("uma0:SceSdstor.txt");
	LogOpen("uma0:SceSdstor.txt");

	LogWrite("text %p, 0x%X\n", sce_info.segments[0].vaddr, sce_info.segments[0].memsz);
	LogWrite("data %p, 0x%X\n", sce_info.segments[1].vaddr, sce_info.segments[1].memsz);
	LogWrite("\n");

	SceSdstorPartInfo *pPartInfo = (SceSdstorPartInfo *)((uintptr_t)sce_info.segments[1].vaddr + 0xAA0);

	for(int i=0;i<15;i++){
		if(pPartInfo[i].data_0x00 == NULL)
			continue;

		LogWrite("0x%X\n", pPartInfo[i].data_0x00);
		LogWrite("0x%X\n", pPartInfo[i].data_0x04);
		LogWrite("id       :%s\n", pPartInfo[i].id);
		LogWrite("flags    :0x%X\n", pPartInfo[i].flags);
		LogWrite("unk_0x10 :0x%X\n", pPartInfo[i].unk_0x10);
		LogWrite("start_lba:0x%X\n", pPartInfo[i].start_lba);
		LogWrite("n_sectors:0x%X\n", pPartInfo[i].n_sectors);
		LogWrite("\n");
	}

	pPartInfo = (SceSdstorPartInfo *)((uintptr_t)sce_info.segments[1].vaddr + 0xC60);

	for(int i=0;i<1;i++){
		if(pPartInfo[i].data_0x00 == NULL)
			continue;

		LogWrite("0x%X\n", pPartInfo[i].data_0x00);
		LogWrite("0x%X\n", pPartInfo[i].data_0x04);
		LogWrite("id       :%s\n", pPartInfo[i].id);
		LogWrite("flags    :0x%X\n", pPartInfo[i].flags);
		LogWrite("unk_0x10 :0x%X\n", pPartInfo[i].unk_0x10);
		LogWrite("start_lba:0x%X\n", pPartInfo[i].start_lba);
		LogWrite("n_sectors:0x%X\n", pPartInfo[i].n_sectors);
		LogWrite("\n");
	}

	pPartInfo = (SceSdstorPartInfo *)((uintptr_t)sce_info.segments[1].vaddr + 0xE98);

	for(int i=0;i<1;i++){
		if(pPartInfo[i].data_0x00 == NULL)
			continue;

		LogWrite("0x%X\n", pPartInfo[i].data_0x00);
		LogWrite("0x%X\n", pPartInfo[i].data_0x04);
		LogWrite("id       :%s\n", pPartInfo[i].id);
		LogWrite("flags    :0x%X\n", pPartInfo[i].flags);
		LogWrite("unk_0x10 :0x%X\n", pPartInfo[i].unk_0x10);
		LogWrite("start_lba:0x%X\n", pPartInfo[i].start_lba);
		LogWrite("n_sectors:0x%X\n", pPartInfo[i].n_sectors);
		LogWrite("\n");
	}

	pPartInfo = (SceSdstorPartInfo *)((uintptr_t)sce_info.segments[1].vaddr + 0xF10);

	for(int i=0;i<2;i++){
		if(pPartInfo[i].data_0x00 == NULL)
			continue;

		LogWrite("0x%X\n", pPartInfo[i].data_0x00);
		LogWrite("0x%X\n", pPartInfo[i].data_0x04);
		LogWrite("id       :%s\n", pPartInfo[i].id);
		LogWrite("flags    :0x%X\n", pPartInfo[i].flags);
		LogWrite("unk_0x10 :0x%X\n", pPartInfo[i].unk_0x10);
		LogWrite("start_lba:0x%X\n", pPartInfo[i].start_lba);
		LogWrite("n_sectors:0x%X\n", pPartInfo[i].n_sectors);
		LogWrite("\n");
	}

	pPartInfo = (SceSdstorPartInfo *)((uintptr_t)sce_info.segments[1].vaddr + 0x10D0);

	for(int i=0;i<1;i++){
		if(pPartInfo[i].data_0x00 == NULL)
			continue;

		LogWrite("0x%X\n", pPartInfo[i].data_0x00);
		LogWrite("0x%X\n", pPartInfo[i].data_0x04);
		LogWrite("id       :%s\n", pPartInfo[i].id);
		LogWrite("flags    :0x%X\n", pPartInfo[i].flags);
		LogWrite("unk_0x10 :0x%X\n", pPartInfo[i].unk_0x10);
		LogWrite("start_lba:0x%X\n", pPartInfo[i].start_lba);
		LogWrite("n_sectors:0x%X\n", pPartInfo[i].n_sectors);
		LogWrite("\n");
	}

	pPartInfo = (SceSdstorPartInfo *)((uintptr_t)sce_info.segments[1].vaddr + 0x1308);

	for(int i=0;i<1;i++){
		if(pPartInfo[i].data_0x00 == NULL)
			continue;

		LogWrite("0x%X\n", pPartInfo[i].data_0x00);
		LogWrite("0x%X\n", pPartInfo[i].data_0x04);
		LogWrite("id       :%s\n", pPartInfo[i].id);
		LogWrite("flags    :0x%X\n", pPartInfo[i].flags);
		LogWrite("unk_0x10 :0x%X\n", pPartInfo[i].unk_0x10);
		LogWrite("start_lba:0x%X\n", pPartInfo[i].start_lba);
		LogWrite("n_sectors:0x%X\n", pPartInfo[i].n_sectors);
		LogWrite("\n");
	}

	LogClose();

	return SCE_KERNEL_START_SUCCESS;
}
