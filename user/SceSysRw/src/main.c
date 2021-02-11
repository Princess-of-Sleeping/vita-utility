/*
 * PSVita SysRw
 * Copyright (C) 2021, Princess of Sleeping
 */

#include <psp2/kernel/modulemgr.h>
#include <psp2/vshbridge.h>

int sceAppMgrDestroyAppByAppId(int appid);

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp){

	int buf[6];

	buf[0] = 0;
	buf[1] = 0;
	buf[2] = 0;
	buf[3] = 0;
	buf[4] = 0;
	buf[5] = 0;

	vshIoUmount(0x200, 1, 0, 0);
	_vshIoMount(0x200, NULL, 2, buf);

	buf[0] = 0;
	buf[1] = 0;
	buf[2] = 0;
	buf[3] = 0;
	buf[4] = 0;
	buf[5] = 0;

	vshIoUmount(0x300, 1, 0, 0);
	_vshIoMount(0x300, NULL, 2, buf);

	sceAppMgrDestroyAppByAppId(~2);

	return SCE_KERNEL_START_SUCCESS;
}
