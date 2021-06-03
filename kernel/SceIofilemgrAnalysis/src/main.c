/*
 * SceIofilemgr Analysis
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

typedef struct SceIofilemgrKpls { // size is 0x28
	SceSize file_open_max_num;
	int cpu_atomic_0x04;
	int data_0x08;
	int cpu_atomic_0x0C;
	int process_status;
	int process_default_priority;
	int data_0x18;
	SceSize dir_open_max_level;
	int cpu_atomic_0x20;
	int cpu_atomic_0x24;
} SceIofilemgrKpls;

int sceIofilemgrAnalysisThread(SceSize args, void *argp){

	ksceKernelDelayThread(17 * 1000 * 1000);

	SceKernelModuleInfo sce_info;
	SceUID moduleid = ksceKernelSearchModuleByName("SceIofilemgr");

	memset(&sce_info, 0, sizeof(sce_info));
	sce_info.size = sizeof(sce_info);
	ksceKernelGetModuleInfo(0x10005, moduleid, &sce_info);

	int KPLS_key = *(int *)((uintptr_t)sce_info.segments[1].vaddr + 0x0);

	int res;

	ksceIoRemove("uma0:SceIofilemgr.txt");
	LogOpen("uma0:SceIofilemgr.txt");

	LogWrite("text %p, 0x%X\n", sce_info.segments[0].vaddr, sce_info.segments[0].memsz);
	LogWrite("data %p, 0x%X\n", sce_info.segments[1].vaddr, sce_info.segments[1].memsz);
	LogWrite("\n");


	SceUID shellpid = ksceKernelSysrootGetShellPid();

	void *pKpls = NULL;
	res = ksceKernelGetProcessLocalStorageAddrForPid(shellpid, KPLS_key, &pKpls, 0);
	if(res >= 0){
		write_file("uma0:SceIofilemgr_Kpls.bin", pKpls, 0x28);
	}else{
		LogWrite("sceKernelGetRemoteKPLS failed : 0x%X\n", res);
	}

	LogClose();




	return 0;
}

/*

int SceIofilemgrForDriver_03F6A684()
*/

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp){

	SceUID thid = ksceKernelCreateThread("SceIofilemgrAnalysisThread", sceIofilemgrAnalysisThread, 0x70, 0x2000, 0, 0, NULL);

	ksceKernelStartThread(thid, 0, NULL);

	return SCE_KERNEL_START_SUCCESS;
}
