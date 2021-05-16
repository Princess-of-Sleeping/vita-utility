/*
 * PSVita VisibleId dumper
 * Copyright (C) 2021, Princess of Sleeping
 */

#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/threadmgr.h>
#include <psp2kern/kernel/debug.h>
#include <psp2kern/kernel/sysclib.h>
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

int run_on_thread(const void *func){

	int ret = 0, res = 0;
	SceUID uid;

	ret = uid = ksceKernelCreateThread("run_on_thread", func, 64, 0x2000, 0, 0, 0);

	if (ret < 0) {
		ksceDebugPrintf("failed to create a thread: 0x%08x\n", ret);
		ret = -1;
		goto cleanup;
	}
	if ((ret = ksceKernelStartThread(uid, 0, NULL)) < 0) {
		ksceDebugPrintf("failed to start a thread: 0x%08x\n", ret);
		ret = -1;
		goto cleanup;
	}
	if ((ret = ksceKernelWaitThreadEnd(uid, &res, NULL)) < 0) {
		ksceDebugPrintf("failed to wait a thread: 0x%08x\n", ret);
		ret = -1;
		goto cleanup;
	}

	ret = res;

cleanup:
	if (uid > 0)
		ksceKernelDeleteThread(uid);

	return ret;
}

typedef struct SceVisibleId {
	char data[0x20];
} SceVisibleId;

int ksceSblAimgrGetVisibleId(SceVisibleId *pVisibleId);

int i(void *args){

	int res = -1;
	const char *dir;
	SceIoStat stat;

	dir = "host0:";

	if(res < 0 && (res = ksceIoGetstat(dir, &stat)) < 0)
		dir = "sd0:";

	if(res < 0 && (res = ksceIoGetstat(dir, &stat)) < 0)
		dir = "ux0:";

	if(res >= 0){
		char path[0x100];
		snprintf(path, sizeof(path), "%s/visible_id.bin", dir);

		SceVisibleId visible_id;
		memset(&visible_id, 0, sizeof(visible_id));
		ksceSblAimgrGetVisibleId(&visible_id);

		write_file(path, &visible_id, sizeof(visible_id));
	}

	return 0;
}

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp){

	run_on_thread(i);

	return SCE_KERNEL_START_NO_RESIDENT;
}
