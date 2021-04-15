/*
 * PSVita IdStorage raw dumper
 * Copyright (C) 2021, Princess of Sleeping
 */

#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/threadmgr.h>
#include <psp2kern/kernel/sysmem.h>
#include <psp2kern/kernel/sysclib.h>
#include <psp2kern/kernel/debug.h>
#include <psp2kern/io/fcntl.h>
#include <psp2kern/io/stat.h>

#define BUFFER_SIZE (0x20000)

char buf[BUFFER_SIZE] __attribute__((aligned(0x40)));

int write_file_from_fd(SceUID read_fd, const char *path, SceSize size){

	int res;
	SceUID fd;

	fd = ksceIoOpen(path, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0666);

	while(size >= BUFFER_SIZE){
		res = ksceIoRead(read_fd, buf, BUFFER_SIZE);
		if(res < 0){
			goto error;
		}

		if(res != BUFFER_SIZE){
			res = -1;
			goto error;
		}

		res = ksceIoWrite(fd, buf, BUFFER_SIZE);
		if(res < 0){
			goto error;
		}

		if(res != BUFFER_SIZE){
			res = -1;
			goto error;
		}

		size -= BUFFER_SIZE;
	}

	if(size > 0){
		res = ksceIoRead(read_fd, buf, size);
		if(res < 0){
			goto error;
		}

		if(res != size){
			res = -1;
			goto error;
		}

		res = ksceIoWrite(fd, buf, size);
		if(res < 0){
			goto error;
		}

		if(res != size){
			res = -1;
			goto error;
		}
	}

	res = 0;

error:
	if(fd >= 0)
		ksceIoClose(fd);

	return 0;
}

int part_dump(const char *device, const char *dst_path){

	int res;
	SceUID fd;

	fd = ksceIoOpen(device, SCE_O_RDONLY, 0777);

	SceOff device_size = ksceIoLseek(fd, 0LL, SCE_SEEK_END);
	if(device_size < 0){
		res = (int)device_size;
		goto error;
	}

	if(device_size > 0xFFFFFFFF){
		res = -1;
		goto error;
	}

	ksceIoLseek(fd, 0LL, SCE_SEEK_SET);

	res = write_file_from_fd(fd, dst_path, (SceSize)device_size);

error:
	if(fd >= 0)
		ksceIoClose(fd);

	return res;
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

int idstorage_dump_main(void){

	const char *drv;
	SceIoStat stat;

	drv = "host0:";

	if(ksceIoGetstat(drv, &stat) < 0)
		drv = "sd0:";

	if(ksceIoGetstat(drv, &stat) < 0)
		drv = "ux0:";

	if(ksceIoGetstat(drv, &stat) < 0){
		ksceDebugPrintf("Idstorage dump writable drive not found.\n");
		return SCE_KERNEL_START_FAILED;
	}

	char path[0x40];

	snprintf(path, sizeof(path), "%s/idstorage_part_raw.bin", drv);
	if(ksceIoGetstat(path, &stat) < 0)
		part_dump("sdstor0:int-lp-ign-idstor", path);

	return 0;
}

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp){

	run_on_thread(idstorage_dump_main);

	return SCE_KERNEL_START_NO_RESIDENT;
}
