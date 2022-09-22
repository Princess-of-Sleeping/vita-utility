/*
 * PSVita IdStorage dumper
 * Copyright (C) 2021, Princess of Sleeping
 */

#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/threadmgr.h>
#include <psp2kern/kernel/sysmem.h>
#include <psp2kern/kernel/sysclib.h>
#include <psp2kern/kernel/debug.h>
#include <psp2kern/io/fcntl.h>
#include <psp2kern/io/stat.h>
#include <psp2kern/idstorage.h>

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


int dump_entry(SceSize args, void *argp){

	char drv[8];
	memset(drv, 0, sizeof(drv));

	SceIoStat stat;

	if(ksceIoGetstat("host0:", &stat) >= 0){
		snprintf(drv, sizeof(drv), "host0:");
	}else if(ksceIoGetstat("sd0:", &stat) >= 0){
		snprintf(drv, sizeof(drv), "sd0:");
	}else if(ksceIoGetstat("ux0:", &stat) >= 0){
		snprintf(drv, sizeof(drv), "ux0:");
	}else{
		ksceDebugPrintf("IdStorage dump writable drive not found.\n");
		return SCE_KERNEL_START_FAILED;
	}

	char path[0x40];

	snprintf(path, sizeof(path), "%s/idstorage_dump", drv);

	if(ksceIoGetstat(path, &stat) >= 0){
		ksceDebugPrintf("Has IdStorage dump already.\n");
		ksceDebugPrintf("If you want dump IdStorage, Remove %s\n", path);
		return SCE_KERNEL_START_NO_RESIDENT;
	}

	SceAllocOpt opt;
	memset(&opt, 0, sizeof(opt));
	opt.size  = sizeof(opt);
	opt.align = 0x40;

	void *leaf_buffer = ksceKernelAllocHeapMemoryWithOption(0x1000B, 0x200, &opt);
	if(leaf_buffer == NULL){
		ksceDebugPrintf("Leaf buffer alloc failed\n");
		return SCE_KERNEL_START_FAILED;
	}

	int res;

	res = ksceIoMkdir(path, 0666);
	if(res < 0){
		ksceDebugPrintf("Failed sceIoMkdir 0x%X. dir:%s\n", res, path);
		ksceKernelFreeHeapMemory(0x1000B, leaf_buffer);
		return SCE_KERNEL_START_FAILED;
	}

	for(int i=0;i<0xFFF0;i++){
		res = ksceIdStorageReadLeaf(i, leaf_buffer);
		if(res >= 0){
			char file[0x80];
			snprintf(file, sizeof(file), "%s/leaf%05d.bin", path, i);
			write_file(file, leaf_buffer, 0x200);
		}
	}

	ksceKernelFreeHeapMemory(0x1000B, leaf_buffer);
	return 0;
}

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp){

	SceUID thid;

	thid = ksceKernelCreateThread("idstorage_dump_thread", dump_entry, 0x50, 0x1000, 0, 0, NULL);
	ksceKernelStartThread(thid, 0, NULL);
	ksceKernelWaitThreadEnd(thid, NULL, NULL);
	ksceKernelDeleteThread(thid);

	return SCE_KERNEL_START_NO_RESIDENT;
}
