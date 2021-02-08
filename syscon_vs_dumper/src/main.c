/*
 * PSVita syscon vs dumper
 * Copyright (C) 2021, CelesteBlue, Princess of Sleeping
 */

#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/sysclib.h>
#include <psp2kern/io/fcntl.h>
#include <psp2kern/syscon.h>

#define sceSysconVsReadData ksceSysconReadCommand

char vs_storage[0x100];

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp) {

	memset(vs_storage, 0, sizeof(vs_storage));

	for(int i=0;i<sizeof(vs_storage);i+=8){
		sceSysconVsReadData(i, &vs_storage[i], 8);
	}

	SceUID fd;

	fd = ksceIoOpen("uma0:syscon_vs_dump.bin", SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0666);
	if(fd < 0)
		fd = ksceIoOpen("sd0:syscon_vs_dump.bin", SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0666);
	if(fd < 0)
		fd = ksceIoOpen("ux0:syscon_vs_dump.bin", SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0666);

	if(fd >= 0){
		ksceIoWrite(fd, vs_storage, sizeof(vs_storage));
		ksceIoClose(fd);
	}

	return SCE_KERNEL_START_NO_RESIDENT;
}
