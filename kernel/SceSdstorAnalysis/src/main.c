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
	uint32_t start_lba;
	uint32_t n_sectors;
	void *data_0x00; // pointer to SceSdstor_data + 0xA40
	ScePartitionEntrie *data_0x04;
	char id[4];
	int flags;
	int unk_0x10;
} SceSdstorPartInfo;

typedef struct SceSdstorDeviceContext { // size is 0x238-bytes
	char fast_mutex_SceSdStorPartitionTable[0x40]; // size is 0x40-bytes

	uint32_t unk_40; // maybe sector size?
	// sd_stor_device *dev;
	void *dev;
	uint32_t unk_48; // maybe sector size?
	uint32_t device_sectors;
	uint32_t unk_50;
	void *mbr_ptr; // pointer to corresponding MBR record in array (offset 0x40)

	SceSdstorPartInfo partitions[17]; // 16 real partition entries from MBR + some hardcoded 17th partition entry
	uint8_t unk_234; // some initialization flag (0, 1, 2)
	uint8_t unk_235;
	uint8_t unk_236; // some initialization flag (set to 0 if partition table is initialized)
	uint8_t unk_237;
} SceSdstorDeviceContext;


char data_seg[0x1EC0];

const char * const sdstor_0[7] = {
	"int",
	"ext",
	"gcd",
	"mcd",
	"uma",
	"usd",
	"xmc"
};

const char * const sdstor_1[2] = {
	"lp",
	"pp"
};

const char * const sdstor_2[3] = {
	"ina",
	"act",
	"ign"
};

const char * const sdstor_3[0x10] = {
	"unused",
	"idstor",
	"sloader",
	"os",

	"vsh",
	"vshdata",
	"vtrm",
	"user",

	"userext",
	"gamero",
	"gamerw",
	"updater",

	"sysdata",
	"mediaid",
	"pidata",
	"entire"
};

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp){

	SceKernelModuleInfo sce_info;
	SceUID moduleid = ksceKernelSearchModuleByName("SceSdstor");


	memset(&sce_info, 0, sizeof(sce_info));
	sce_info.size = sizeof(sce_info);
	ksceKernelGetModuleInfo(0x10005, moduleid, &sce_info);

	int (* FUN_81001b80)(const char *src, SceSize len, char *dst);

	FUN_81001b80 = sce_info.segments[0].vaddr + (0x1b80 | 1);


	SceSdstorMbr *pMbr = (SceSdstorMbr *)((uintptr_t)sce_info.segments[1].vaddr + 0x40);

	char path[0x80];

	for(int i=0;i<5;i++){
		snprintf(path, sizeof(path), "uma0:sdstor_mbr_%d.bin", i);
		// write_file(path, &pMbr[i], 0x200);
	}

	if(0){
		memcpy(data_seg, sce_info.segments[1].vaddr, sizeof(data_seg));
		write_file("host0:a", data_seg, sizeof(data_seg));
	}

	SceSdstorDeviceContext *pContext = (SceSdstorDeviceContext *)((uintptr_t)sce_info.segments[1].vaddr + 0xA40);


	/*
	 * SceSdstor_data + 0xA40 is same format to SceExfatfs_data + 0x295DE8.
	 * size is 0x40 byte.
	 */

	ksceIoRemove("host0:SceSdstor.txt");
	LogOpen("host0:SceSdstor.txt");

	LogWrite("text %p, 0x%X\n", sce_info.segments[0].vaddr, sce_info.segments[0].memsz);
	LogWrite("data %p, 0x%X\n", sce_info.segments[1].vaddr, sce_info.segments[1].memsz);
	LogWrite("\n");

	LogWrite("context size : 0x%X\n", sizeof(*pContext));
	LogWrite("\n");


	char sdstor_str[0x20];
	char decode_path[0x10];

	for(int s0=0;s0<7;s0++){
		for(int s1=0;s1<2;s1++){
			for(int s2=0;s2<3;s2++){
				for(int s3=0;s3<0x10;s3++){

					snprintf(sdstor_str, sizeof(sdstor_str), "%s-%s-%s-%s", sdstor_0[s0], sdstor_1[s1], sdstor_2[s2], sdstor_3[s3]);
					int res = FUN_81001b80(sdstor_str, strlen(sdstor_str), decode_path);
					if(res >= 0){
						LogWrite("0x%X 0x%X 0x%X 0x%X %s\n", decode_path[0], decode_path[1], decode_path[2], decode_path[3], sdstor_str);
					}else{
						LogWrite("0x%X %s\n", res, sdstor_str);
					}
				}
			}
		}
	}



	for(int i=0;i<5;i++){

		LogWrite("%d\n", i);
		LogWrite("\tunk_40: 0x%X\n", pContext[i].unk_40);
		LogWrite("\tdev: 0x%X\n", pContext[i].dev);
		LogWrite("\tunk_48: 0x%X\n", pContext[i].unk_48);
		LogWrite("\tdevice_sectors: 0x%X (%lld)\n", pContext[i].device_sectors, (SceUInt64)pContext[i].device_sectors * (SceUInt64)pContext[i].unk_40);
		LogWrite("\tunk_50: 0x%X\n", pContext[i].unk_50);
		LogWrite("\tmbr_ptr: 0x%X\n", pContext[i].mbr_ptr);

		for(int n=0;n<17;n++){
			LogWrite("\t\tdata_0x00: 0x%X\n", pContext[i].partitions[n].data_0x00);
			LogWrite("\t\tdata_0x04: 0x%X\n", pContext[i].partitions[n].data_0x04);
			LogWrite("\t\tid: %s\n", pContext[i].partitions[n].id);
			LogWrite("\t\tflags: 0x%X\n", pContext[i].partitions[n].flags);
			LogWrite("\t\tunk_0x10: 0x%X\n", pContext[i].partitions[n].unk_0x10);
			LogWrite("\t\tstart_lba: 0x%X\n", pContext[i].partitions[n].start_lba);
			LogWrite("\t\tn_sectors: 0x%X (%lld)\n", pContext[i].partitions[n].n_sectors, (SceUInt64)pContext[i].partitions[n].n_sectors * (SceUInt64)pContext[i].unk_40);
			LogWrite("\n");
		}

		LogWrite("\tunk_234: 0x%X\n", pContext[i].unk_234);
		LogWrite("\tunk_235: 0x%X\n", pContext[i].unk_235);
		LogWrite("\tunk_236: 0x%X\n", pContext[i].unk_236);
		LogWrite("\tunk_237: 0x%X\n", pContext[i].unk_237);
		LogWrite("\n");
	}

	LogClose();

	return SCE_KERNEL_START_NO_RESIDENT;
}
