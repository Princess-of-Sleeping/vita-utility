/*
 * PSVita Kernel Module Loader
 * Copyright (C) 2021, Princess of Sleeping
 */

#include <psp2/kernel/modulemgr.h>
#include <psp2/kernel/threadmgr.h>
#include <taihen.h>

int sceAppMgrDestroyAppByAppId(int appid);

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp){

	SceUID modid = -1;

	if(modid < 0)
		modid = taiLoadStartKernelModule("host0:module.skprx", 0, NULL, 0);
	if(modid < 0)
		modid = taiLoadStartKernelModule("host0:baremetal-loader.skprx", 0, NULL, 0);
	if(modid < 0)
		modid = taiLoadStartKernelModule("host0:kbl-loader.skprx", 0, NULL, 0);

	if(modid < 0)
		modid = taiLoadStartKernelModule("ux0:module.skprx", 0, NULL, 0);
	if(modid < 0)
		modid = taiLoadStartKernelModule("ux0:kbl-loader.skprx", 0, NULL, 0);

	sceKernelDelayThread(1000000);

	sceAppMgrDestroyAppByAppId(~2);

	return SCE_KERNEL_START_SUCCESS;
}
