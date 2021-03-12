/*
 * PSVita Kbl param dumper
 * Copyright (C) 2021, Princess of Sleeping
 */

#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/sysroot.h>
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

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp){

	SceIoStat stat;

	if(ksceIoGetstat("host0:", &stat) >= 0){
		write_file("host0:kblparam.bin", ksceKernelSysrootGetKblParam(), 0x200);
	}else if(ksceIoGetstat("sd0:", &stat) >= 0){
		write_file("sd0:kblparam.bin", ksceKernelSysrootGetKblParam(), 0x200);
	}else if(ksceIoGetstat("ux0:", &stat) >= 0){
		write_file("ux0:kblparam.bin", ksceKernelSysrootGetKblParam(), 0x200);
	}

	return SCE_KERNEL_START_NO_RESIDENT;
}
