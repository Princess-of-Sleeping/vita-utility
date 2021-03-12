/*
 * PSVita Non-volatile storage dumper
 * Copyright (C) 2021, Princess of Sleeping
 */

#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/sysmem.h>
#include <psp2kern/kernel/sysclib.h>
#include <psp2kern/kernel/debug.h>
#include <psp2kern/io/fcntl.h>
#include <psp2kern/io/stat.h>

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

int ksceSblNvsReadData(SceUInt32 offset, void *buffer, SceSize size);

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp){

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
		ksceDebugPrintf("NVS dump writable drive not found.\n");
		return SCE_KERNEL_START_FAILED;
	}

	char path[0x40];

	snprintf(path, sizeof(path), "%s/NVS_dump.bin", drv);

	if(ksceIoGetstat(path, &stat) >= 0){
		ksceDebugPrintf("Has NVS dump already.\n");
		ksceDebugPrintf("If you want dump NVS, Remove %s\n", path);
		return SCE_KERNEL_START_NO_RESIDENT;
	}

	SceAllocOpt opt;
	memset(&opt, 0, sizeof(opt));
	opt.size  = sizeof(opt);
	opt.align = 0x40;

	void *nvs_buffer = ksceKernelAllocHeapMemoryWithOption(0x1000B, 0xB60, &opt);
	if(nvs_buffer == NULL){
		ksceDebugPrintf("NVS buffer alloc failed\n");
		return SCE_KERNEL_START_FAILED;
	}

	memset(nvs_buffer, 0, 0xB60);

	int res;

	/*
	 * offset before 0x400 is Secure NVS. cannot read in non-secure kernel.
	 */
	for(int i=0x400;i<0xB60;i+=0x20){
		res = ksceSblNvsReadData(i, (void *)(((uintptr_t)nvs_buffer) + i), 0x20);
		if(res < 0){
			ksceDebugPrintf("Failed sceSblNvsReadData 0x%X. offset:0x%04X\n", res, i);
		}
	}

	write_file(path, nvs_buffer, 0xB60);

	ksceKernelFreeHeapMemory(0x1000B, nvs_buffer);

	return SCE_KERNEL_START_NO_RESIDENT;
}
