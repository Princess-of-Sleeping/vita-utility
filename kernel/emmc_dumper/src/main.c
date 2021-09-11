/*
 * PSVita emmc dumper
 * Copyright (C) 2021, Princess of Sleeping
 */

#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/threadmgr.h>
#include <psp2kern/kernel/sysmem.h>
#include <psp2kern/kernel/sysclib.h>
#include <psp2kern/kernel/debug.h>
#include <psp2kern/kernel/debugled.h>
#include <psp2kern/io/fcntl.h>
#include <psp2kern/io/stat.h>
#include <psp2kern/display.h>

#define BUFFER_SIZE (0x80000)

char buf[BUFFER_SIZE] __attribute__((aligned(0x40)));

int write_file_from_fd(SceUID read_fd, const char *path, SceSize size){

	int res;
	SceUID fd;

	fd = ksceIoOpen(path, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0666);
	if(fd < 0){
		ksceDebugPrintf("%s:Failed %s (0x%X)\n", __FUNCTION__, "sceIoOpen", fd);
		return fd;
	}

#define SCE_CST_SIZE        0x0004

	SceIoStat stat;
	memset(&stat, 0, sizeof(stat));
	stat.st_size = (SceOff)size;

	res = ksceIoChstatByFd(fd, &stat, SCE_CST_SIZE);
	if(res < 0){
		ksceDebugPrintf("%s:Failed %s (0x%X)\n", __FUNCTION__, "sceIoChstatByFd", res);
		ksceIoClose(fd);
		return res;
	}

	ksceIoLseek(fd, 0LL, SCE_SEEK_SET);

	int gpo_mask = 0;

	while(size >= BUFFER_SIZE){

		ksceKernelSetGPO(gpo_mask);
		gpo_mask ^= 1;

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

	ksceKernelSetGPO(0);

	return 0;
}

int part_dump(const char *device, const char *dst_path){

	int res;
	SceUID fd;

	fd = ksceIoOpen(device, SCE_O_RDONLY, 0777);
	if(fd < 0){
		ksceDebugPrintf("Not found : %s\n", device);
		return -1;
	}

	ksceDebugPrintf("Start dump %s\n", device);

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

int emmc_dump_main(void){

	const char *drv;
	char dir[0x10];
	SceIoStat stat;

	drv = "host0:";

	if(ksceIoGetstat(drv, &stat) < 0)
		drv = "sd0:";

	if(ksceIoGetstat(drv, &stat) < 0)
		drv = "ux0:";

	snprintf(dir, sizeof(dir) - 1, "%s/emmc", drv);
	drv = dir;

	if(ksceIoGetstat(drv, &stat) < 0){
		if(ksceIoMkdir(drv, 0666) < 0){
			ksceDebugPrintf("Emmc dump writable drive not found.\n");
			return SCE_KERNEL_START_FAILED;
		}
	}

	char path[0x40];

	snprintf(path, sizeof(path), "%s/idstorage_part_raw.bin", drv);
	if(ksceIoGetstat(path, &stat) < 0)
		part_dump("sdstor0:int-lp-ign-idstor", path);
/*
	snprintf(path, sizeof(path), "%s/idstorage_part_raw_act.bin", drv);
	if(ksceIoGetstat(path, &stat) < 0)
		part_dump("sdstor0:int-lp-act-idstor", path);

	snprintf(path, sizeof(path), "%s/idstorage_part_raw_ina.bin", drv);
	if(ksceIoGetstat(path, &stat) < 0)
		part_dump("sdstor0:int-lp-ina-idstor", path);
*/


	snprintf(path, sizeof(path), "%s/slb2_raw_act.bin", drv);
	if(ksceIoGetstat(path, &stat) < 0)
		part_dump("sdstor0:int-lp-act-sloader", path);

	snprintf(path, sizeof(path), "%s/slb2_raw.bin", drv);
	if(ksceIoGetstat(path, &stat) < 0)
		part_dump("sdstor0:int-lp-ign-sloader", path);
/*
	snprintf(path, sizeof(path), "%s/slb2_raw_ina.bin", drv);
	if(ksceIoGetstat(path, &stat) < 0)
		part_dump("sdstor0:int-lp-ina-sloader", path);
*/


	/*
	 * os0:
	 */

	snprintf(path, sizeof(path), "%s/os0_raw_act.bin", drv);
	if(ksceIoGetstat(path, &stat) < 0)
		part_dump("sdstor0:int-lp-act-os", path);
/*
	snprintf(path, sizeof(path), "%s/os0_raw.bin", drv);
	if(ksceIoGetstat(path, &stat) < 0)
		part_dump("sdstor0:int-lp-ign-os", path);

	snprintf(path, sizeof(path), "%s/os0_raw_ina.bin", drv);
	if(ksceIoGetstat(path, &stat) < 0)
		part_dump("sdstor0:int-lp-ina-os", path);
*/


	snprintf(path, sizeof(path), "%s/vs0_raw.bin", drv);
	if(ksceIoGetstat(path, &stat) < 0)
		part_dump("sdstor0:int-lp-ign-vsh", path);
/*
	snprintf(path, sizeof(path), "%s/vs0_raw_act.bin", drv);
	if(ksceIoGetstat(path, &stat) < 0)
		part_dump("sdstor0:int-lp-act-vsh", path);

	snprintf(path, sizeof(path), "%s/vs0_raw_ina.bin", drv);
	if(ksceIoGetstat(path, &stat) < 0)
		part_dump("sdstor0:int-lp-ina-vsh", path);
*/



	snprintf(path, sizeof(path), "%s/vd0_raw.bin", drv);
	if(ksceIoGetstat(path, &stat) < 0)
		part_dump("sdstor0:int-lp-ign-vshdata", path);
/*
	snprintf(path, sizeof(path), "%s/vd0_raw_act.bin", drv);
	if(ksceIoGetstat(path, &stat) < 0)
		part_dump("sdstor0:int-lp-act-vshdata", path);

	snprintf(path, sizeof(path), "%s/vd0_raw_ina.bin", drv);
	if(ksceIoGetstat(path, &stat) < 0)
		part_dump("sdstor0:int-lp-ina-vshdata", path);
*/


	snprintf(path, sizeof(path), "%s/tm0_raw.bin", drv);
	if(ksceIoGetstat(path, &stat) < 0)
		part_dump("sdstor0:int-lp-ign-vtrm", path);
/*
	snprintf(path, sizeof(path), "%s/tm0_raw_act.bin", drv);
	if(ksceIoGetstat(path, &stat) < 0)
		part_dump("sdstor0:int-lp-act-vtrm", path);

	snprintf(path, sizeof(path), "%s/tm0_raw_ina.bin", drv);
	if(ksceIoGetstat(path, &stat) < 0)
		part_dump("sdstor0:int-lp-ina-vtrm", path);
*/



	snprintf(path, sizeof(path), "%s/ur0_raw.bin", drv);
	if(ksceIoGetstat(path, &stat) < 0)
		part_dump("sdstor0:int-lp-ign-user", path);
/*
	snprintf(path, sizeof(path), "%s/ur0_raw_act.bin", drv);
	if(ksceIoGetstat(path, &stat) < 0)
		part_dump("sdstor0:int-lp-act-user", path);

	snprintf(path, sizeof(path), "%s/ur0_raw_ina.bin", drv);
	if(ksceIoGetstat(path, &stat) < 0)
		part_dump("sdstor0:int-lp-ina-user", path);
*/



	snprintf(path, sizeof(path), "%s/imc0_raw.bin", drv);
	if(ksceIoGetstat(path, &stat) < 0)
		part_dump("sdstor0:int-lp-ign-userext", path);
/*
	snprintf(path, sizeof(path), "%s/imc0_raw_act.bin", drv);
	if(ksceIoGetstat(path, &stat) < 0)
		part_dump("sdstor0:int-lp-act-userext", path);

	snprintf(path, sizeof(path), "%s/imc0_raw_ina.bin", drv);
	if(ksceIoGetstat(path, &stat) < 0)
		part_dump("sdstor0:int-lp-ina-userext", path);
*/




	snprintf(path, sizeof(path), "%s/ud0_raw.bin", drv);
	if(ksceIoGetstat(path, &stat) < 0)
		part_dump("sdstor0:int-lp-ign-updater", path);
/*
	snprintf(path, sizeof(path), "%s/ud0_raw_act.bin", drv);
	if(ksceIoGetstat(path, &stat) < 0)
		part_dump("sdstor0:int-lp-act-updater", path);

	snprintf(path, sizeof(path), "%s/ud0_raw_ina.bin", drv);
	if(ksceIoGetstat(path, &stat) < 0)
		part_dump("sdstor0:int-lp-ina-updater", path);
*/


	snprintf(path, sizeof(path), "%s/sa0_raw.bin", drv);
	if(ksceIoGetstat(path, &stat) < 0)
		part_dump("sdstor0:int-lp-ign-sysdata", path);
/*
	snprintf(path, sizeof(path), "%s/sa0_raw_act.bin", drv);
	if(ksceIoGetstat(path, &stat) < 0)
		part_dump("sdstor0:int-lp-act-sysdata", path);

	snprintf(path, sizeof(path), "%s/sa0_raw_ina.bin", drv);
	if(ksceIoGetstat(path, &stat) < 0)
		part_dump("sdstor0:int-lp-ina-sysdata", path);
*/


	snprintf(path, sizeof(path), "%s/pd0_raw.bin", drv);
	if(ksceIoGetstat(path, &stat) < 0)
		part_dump("sdstor0:int-lp-ign-pidata", path);
/*
	snprintf(path, sizeof(path), "%s/pd0_raw_act.bin", drv);
	if(ksceIoGetstat(path, &stat) < 0)
		part_dump("sdstor0:int-lp-act-pidata", path);

	snprintf(path, sizeof(path), "%s/pd0_raw_ina.bin", drv);
	if(ksceIoGetstat(path, &stat) < 0)
		part_dump("sdstor0:int-lp-ina-pidata", path);
*/

	snprintf(path, sizeof(path), "%s/nand_raw_act.bin", drv);
	if(ksceIoGetstat(path, &stat) < 0)
		part_dump("sdstor0:int-lp-act-entire", path);
/*
	snprintf(path, sizeof(path), "%s/nand_raw.bin", drv);
	if(ksceIoGetstat(path, &stat) < 0)
		part_dump("sdstor0:int-lp-ign-entire", path);

	snprintf(path, sizeof(path), "%s/nand_raw_ina.bin", drv);
	if(ksceIoGetstat(path, &stat) < 0)
		part_dump("sdstor0:int-lp-ina-entire", path);
*/


	return 0;
}

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp){

	ksceKernelSetGPO(0xC3);
	ksceDisplaySetFrameBuf(NULL, SCE_DISPLAY_SETBUF_NEXTFRAME);

	run_on_thread(emmc_dump_main);

	return SCE_KERNEL_START_NO_RESIDENT;
}
