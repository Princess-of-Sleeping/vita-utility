/*
 * PSVita SLB2 dumper
 * Copyright (C) 2021, Princess of Sleeping
 */

#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/threadmgr.h>
#include <psp2kern/kernel/sysmem.h>
#include <psp2kern/kernel/sysclib.h>
#include <psp2kern/kernel/debug.h>
#include <psp2kern/kernel/utils.h>
#include <psp2kern/io/fcntl.h>
#include <psp2kern/io/stat.h>

typedef struct SceSlb2Entry { // size is 0x30
	uint32_t sector_pos;
	uint32_t size;
	uint32_t unk_0x8;
	uint32_t unk_0xC;
	char file_name[0x20];
} __attribute__((packed)) SceSlb2Entry;

typedef struct SceSlb2Header { // size is 0x200
	char magic[4];
	SceInt32 version;
	SceSize  header_size;
	uint32_t file_count;
	uint32_t file_align;
	uint32_t unk[3];
	SceSlb2Entry entry[10];
} __attribute__((packed)) SceSlb2Header;

#define BUFFER_SIZE (0x20000)

char buf[BUFFER_SIZE] __attribute__((aligned(0x40)));

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

int get_sha256_from_fd(SceUID read_fd, SceSize size, void *hash){

	int res;
	SceSha256Context ctx;
	memset(&ctx, 0, sizeof(ctx));

	ksceSha256BlockInit(&ctx);

	while(size >= BUFFER_SIZE){
		res = ksceIoRead(read_fd, buf, BUFFER_SIZE);
		if(res < 0){
			goto error;
		}

		if(res != BUFFER_SIZE){
			res = -1;
			goto error;
		}

		ksceSha256BlockUpdate(&ctx, buf, BUFFER_SIZE);
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

		ksceSha256BlockUpdate(&ctx, buf, size);
	}

	ksceSha256BlockResult(&ctx, hash);
	res = 0;

error:
	return res;
}

int slb2_extraction(const char *src_slb2, const char *dst_dir){

	int res = 0;
	SceUID fd;
	char path[0x100];
	SceSlb2Header slb2_header;

	fd = ksceIoOpen(src_slb2, SCE_O_RDONLY, 0);
	if(fd < 0){
		res = fd;
		goto end;
	}

	ksceIoLseek(fd, 0, SCE_SEEK_SET);
	ksceIoRead(fd, &slb2_header, sizeof(slb2_header));

	if(memcmp(slb2_header.magic, "SLB2", 4) != 0)
		goto end;

	for(int i=0;i<slb2_header.file_count;i++){
		ksceDebugPrintf(
			"%-31s offset:0x%08X size:0x%08X\n",
			slb2_header.entry[i].file_name, slb2_header.entry[i].sector_pos << 9, slb2_header.entry[i].size
		);

		snprintf(path, sizeof(path), "%s/%s", dst_dir, slb2_header.entry[i].file_name);

		ksceIoLseek(fd, slb2_header.entry[i].sector_pos << 9, SCE_SEEK_SET);
		write_file_from_fd(fd, path, slb2_header.entry[i].size);
	}

end:
	if(fd >= 0)
		ksceIoClose(fd);

	return res;
}

int slb2_check_sha256(const char *src_slb2){

	int res = 0;
	SceUID fd;
	SceSlb2Header slb2_header;

	fd = ksceIoOpen(src_slb2, SCE_O_RDONLY, 0);
	if(fd < 0){
		res = fd;
		goto end;
	}

	ksceIoLseek(fd, 0, SCE_SEEK_SET);
	ksceIoRead(fd, &slb2_header, sizeof(slb2_header));

	if(memcmp(slb2_header.magic, "SLB2", 4) != 0)
		goto end;

	SceSize slb2_size = 0;

	slb2_size += (slb2_header.entry[slb2_header.file_count - 1].sector_pos << 9);
	slb2_size += (slb2_header.entry[slb2_header.file_count - 1].size + 0x1FF) & ~0x1FF;

	char sha256_hash[0x20];

	ksceIoLseek(fd, 0, SCE_SEEK_SET);
	get_sha256_from_fd(fd, slb2_size, sha256_hash);

	ksceDebugPrintf("SHA256:");
	for(int i=0;i<0x20;i++)
		ksceDebugPrintf("%02X", sha256_hash[i]);

	ksceDebugPrintf("\n");

	char path[0x100];

	snprintf(path, sizeof(path), "%s.sha256", src_slb2);
	write_file(path, sha256_hash, sizeof(sha256_hash));

end:
	if(fd >= 0)
		ksceIoClose(fd);

	return res;
}

int slb2_dump(const char *device, const char *dst_path){

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

int slb2_dump_main(void){

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
		ksceDebugPrintf("Slb2 dump writable drive not found.\n");
		return SCE_KERNEL_START_FAILED;
	}

	char path[0x40];
	char dir[0x40];

	snprintf(path, sizeof(path), "%s/slb2_ina.bin", drv);
	if(ksceIoGetstat(path, &stat) < 0)
		slb2_dump("sdstor0:int-lp-ina-sloader", path);

	snprintf(path, sizeof(path), "%s/slb2_act.bin", drv);
	if(ksceIoGetstat(path, &stat) < 0)
		slb2_dump("sdstor0:int-lp-act-sloader", path);

	snprintf(dir, sizeof(dir), "%s/slb2_ina", drv);
	if(ksceIoGetstat(dir, &stat) < 0){
		ksceIoMkdir(dir, 0666);

		snprintf(path, sizeof(path), "%s/slb2_ina.bin", drv);
		slb2_check_sha256(path);
		slb2_extraction(path, dir);
	}

	snprintf(dir, sizeof(dir), "%s/slb2_act", drv);
	if(ksceIoGetstat(dir, &stat) < 0){
		ksceIoMkdir(dir, 0666);

		snprintf(path, sizeof(path), "%s/slb2_act.bin", drv);
		slb2_check_sha256(path);
		slb2_extraction(path, dir);
	}

	return 0;
}

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp){

	run_on_thread(slb2_dump_main);

	return SCE_KERNEL_START_NO_RESIDENT;
}
