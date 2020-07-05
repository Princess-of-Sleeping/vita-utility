/*
 * SceVfs mount PoC
 * Copyright (C) 2020 Princess of Sleeping
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/sysmem.h>
#include <psp2kern/io/dirent.h>
#include <string.h>
#include <stdio.h>
#include "vfs.h"

typedef struct SceVfsMount2 { // size is 0x14
	const char *unit;     // ex:"host0:"
	const char *device1;  // ex:"bsod_dummy_host_fs"
	const char *device2;  // ex:"bsod_dummy_host_fs"
	int data_0x0C;        // ex:0
	int data_0x10;        // ex:0
} SceVfsMount2;

typedef struct SceVfsMount {   // size is 0x20
	const char *device;    // ex:"/host"
	int data_0x04;
	int data_0x08;         // ex:0x03000004
	int data_0x0C;         // ex:0x00008006
	const char *data_0x10; // ex:"bsod_dummy_host_fs"
	int data_0x14;
	const SceVfsMount2 *data_0x18;
	int data_0x1C;
} SceVfsMount;

typedef struct SceVfsUmount {
	const char *device; // ex:"/host"
	int data_0x04;
} SceVfsUmount;

typedef struct SceVfsTable { // size is 0x34?
	const void *func00;
	const void *func04;
	const void *func08;
	const void *func0C;
	const void *func10;
	const void *func14;
	const void *func18;
	const void *func1C;
	const void *func20;
	const void *func24;
	const void *func28;
	const void *func2C;
	const void *func30;
} SceVfsTable;

typedef struct SceVfsTable2 { // size is 0x74?
	const void *func00;
	const void *func04;
	const void *func08;
	const void *func0C;
	const void *func10;
	const void *func14;
	const void *func18;
	const void *func1C;
	const void *func20;
	const void *func24;
	const void *func28;
	const void *func2C;
	const void *func30;
	const void *func34;
	const void *func38;
	const void *func3C;
	const void *func40;
	const void *func44;
	const void *func48;
	const void *func4C;
	const void *func50;
	const void *func54;
	const void *func58;
	const void *func5C;
	const void *func60;
	const void *func64;
	const void *func68;
	const void *func6C;
	const void *func70;
} SceVfsTable2;

typedef struct SceVfsAdd {     // size is 0x1C
	const SceVfsTable *func_ptr1;
	const char *device;    // ex:"bsod_dummy_host_fs"
	int data_0x08;         // ex:0x11
	int data_0x0C;
	int data_0x10;         // ex:0x10
	const SceVfsTable2 *func_ptr2;
	int data_0x18;
} SceVfsAdd;

int ksceVfsMount(const SceVfsMount *pVfsMount);
int ksceVfsUnmount(const SceVfsUmount *pVfsUmount);

int ksceVfsAddVfs(SceVfsAdd *pVfsAdd);
int ksceVfsDeleteVfs(const char *fs, void *a2); // "deci4p_drfp_dev_fs"

int ksceKernelGetRandomNumber(void *dst, SceSize size);

const SceVfsMount2 vfs_mount2 = {
	.unit      = "host0:",
	.device1   = "faps_dummy_host_fs",
	.device2   = "faps_dummy_host_fs",
	.data_0x0C = 0,
	.data_0x10 = 0
};

const SceVfsMount vfs_mount = {
	.device    = "/host",
	.data_0x04 = 0,
	.data_0x08 = 0x03000004,
	.data_0x0C = 0x00008006,
	.data_0x10 = "faps_dummy_host_fs",
	.data_0x14 = 0,
	.data_0x18 = &vfs_mount2,
	.data_0x1C = 0
};

const SceVfsUmount vfs_umount = {
	.device = "/host",
	.data_0x04 = 0
};

SceVfsAdd vfs_add;

void sub_81002110(const char *path){
	ksceDebugPrintf("%s\n", path);
}

int sub_81001DEC(void *a1){

	*(uint32_t *)(*(uint32_t *)(a1) + 0x68) = 0x80;
	*(uint32_t *)(*(uint32_t *)(a1) + 0xC0) = 0x10000;

	return 0;
}

int sub_81001DFE(void *a1){
	return 0;
}

int sub_81001E02(void *a1){

	*(uint32_t *)(*(uint32_t *)(a1 + 8) + 0x48) = *(uint32_t *)(*(uint32_t *)(a1) + 0xC4);
	*(uint32_t *)(*(uint32_t *)(a1 + 8) + 0x4C) = *(uint32_t *)(a1);
	*(uint32_t *)(*(uint32_t *)(a1 + 8) + 0x50) = 0;
	*(uint32_t *)(*(uint32_t *)(a1 + 8) + 0x60) = 0;
	*(uint32_t *)(*(uint32_t *)(a1 + 8) + 0x64) = 0;
	*(uint32_t *)(*(uint32_t *)(a1 + 8) + 0x74) = 1;
	*(uint32_t *)(*(uint32_t *)(a1 + 8) + 0x78) = 0x1002;
	*(uint32_t *)(*(uint32_t *)(a1 + 8) + 0x80) = 0;
	*(uint32_t *)(*(uint32_t *)(a1 + 8) + 0x84) = 0;
	*(uint32_t *)(*(uint32_t *)(a1 + 8) + 0x88) = 0;
	*(uint32_t *)(*(uint32_t *)(a1 + 8) + 0x8C) = 0;

	return 0;
}

int sub_81001E2E(void *a1){
	return 0;
}

int sub_81001DE8(void *a1){
	return 0;
}

int sub_81001E32(void *a1){
	return 0x80010086;
}

typedef struct SceVfsPathElem { // size is 0x1C
	void *data_0x00;
	char *path;
	void *data_0x08;
	void *data_0x0C;
	void *data_0x10;
	int data_0x14;
	void *data_0x18;
} SceVfsPathElem;

// call from ksceVfsOpDecodePathElem
int sub_8100222C(SceVfsPathElem *a1){
	return 0;
}

SceSize file_size = 0;

// call from sceVopOpen, a1 size is 0x10(sceVopOpen four args)
int vop_open(void *a1){

	sub_81002110((const char *)*(uint32_t *)(*(uint32_t *)(a1 + 4)));

	file_size = 0x100;

	return 0;
}

int sub_81001E42(void *a1){
	ksceDebugPrintf("sub_81001E42\n");
	return 0x80010086;
}

typedef struct SceVfsClose { // size is 0x8
	void *data_0x00;
	void *data_0x04;
} SceVfsClose;

// call from sceVopClose
int vop_close(SceVfsClose *a1){
	file_size = 0;
	return 0;
}

// call from SceIofilemgrForDriver_A5A6A55C(sceVfsNodeInitializePartition), a1 size is 0x10
int vfs_part_init(SceVfsNodePartInitArgs *a1){

	int res;
	SceVfsNode *pNode = NULL;
	const char *path;

	path = *(const char **)(a1->opt + 8);

	ksceDebugPrintf("%s\n", path);

	if((a1->flags & 3) != 1)
		return 0x80010016;

	if(a1->node->mount == NULL)
		return 0x80010016;

	res = ksceVfsGetNewNode(a1->node->mount, *(uint32_t *)(*(uint32_t *)(a1->node->mount + 0x5C) + 0x14), 0, &pNode);
	if(res < 0){
		ksceDebugPrintf("sceVfsGetNewNode : 0x%X\n", res);
		return res;
	}

	if(pNode == NULL)
		return 0x80010016;

	ksceVfsNodeWaitEventFlag(pNode);

	pNode->dev_info  = NULL;
	pNode->mount     = a1->node->mount;
	pNode->prev_node = a1->node;
	pNode->unk_74    = 1;
	pNode->unk_78    = 0x2010;

	*a1->new_node = pNode;

	return 0;
}

typedef struct SceVfsRead { // size is 0x14
	int data_0x00;
	int data_0x04;
	void *data;
	SceSize size;
	int data_0x10;
} SceVfsRead;

// call from sceVopRead
int vop_read(SceVfsRead *a1){

	int res;

	if(a1->size >= file_size){
		res = file_size;
		file_size = 0;
	}else{
		res = a1->size;
		file_size -= a1->size;
	}

	if(res != 0)
		ksceKernelGetRandomNumber(a1->data, res);

	return res;
}

// call from sceVopWrite
int sub_81001E5A(void *a1){
	ksceDebugPrintf("sub_81001E5A\n");
	return 0x80010086;
}

/*
 * maybe lseek
 */
uint64_t sub_810020C8(void *a1){
	ksceDebugPrintf("sub_810020C8\n");
	return 0xFFFFFFFF80010086;
}

int sub_81002208(void *a1){
	ksceDebugPrintf("sub_81002208\n");
	sub_81002110((const char *)*(uint32_t *)(*(uint32_t *)(a1 + 8)));
	return 0x80010086;
}

int sub_810021F6(void *a1){
	ksceDebugPrintf("sub_810021F6\n");
	sub_81002110((const char *)*(uint32_t *)(*(uint32_t *)(a1 + 8)));
	return 0x80010086;
}

int sub_810021E4(void *a1){
	ksceDebugPrintf("sub_810021E4\n");
	sub_81002110((const char *)*(uint32_t *)(*(uint32_t *)(a1 + 8)));
	return 0x80010086;
}

int dopen_flags = -1;

// call from sceVopDopen, a1 size is 0xC(sceVopDopen three args)
int vop_dopen(void *a1){

	const char *path;

	path = **(const char ***)(a1 + 4);

	if(strcmp(path, "host/.") == 0){
		dopen_flags = 1;
		return 0;
	}

	return 0x80010086;
}

// call from sceVopDclose, a1 size is 0x8(sceVopDclose two args)
int vop_dclose(void *a1){

	dopen_flags = -1;

	return 0x80010086;
}

typedef struct SceVfsVopDread { // size is 0xC
	void *vfs_node;
	void *data_0x04;
	SceIoDirent *dir;
} SceVfsVopDread;

// call from sceVopDread, a1 size is 0xC(sceVopDread three args)
int vop_dread(SceVfsVopDread *argp){

	if(dopen_flags > 0){

		dopen_flags--;

		argp->dir->d_stat.st_mode = SCE_S_IFREG | SCE_S_IROTH | SCE_S_IRUSR | SCE_S_IWUSR;
		argp->dir->d_stat.st_attr = SCE_SO_IFREG;
		argp->dir->d_stat.st_size = 0x100LL;

		argp->dir->d_stat.st_ctime.year        = 2020;
		argp->dir->d_stat.st_ctime.month       = 7;
		argp->dir->d_stat.st_ctime.day         = 5;
		argp->dir->d_stat.st_ctime.hour        = 1;
		argp->dir->d_stat.st_ctime.minute      = 33;
		argp->dir->d_stat.st_ctime.second      = 0;
		argp->dir->d_stat.st_ctime.microsecond = 0;

		memcpy(&argp->dir->d_stat.st_atime, &argp->dir->d_stat.st_ctime, sizeof(SceDateTime));
		memcpy(&argp->dir->d_stat.st_mtime, &argp->dir->d_stat.st_ctime, sizeof(SceDateTime));

		snprintf(argp->dir->d_name, 255, "file%d", dopen_flags);
		argp->dir->d_private = NULL;
		argp->dir->dummy = 0;

		return 1;
	}else if(dopen_flags == 0)
		return 0;

	return 0x80010086;
}

typedef struct SceVfsVopStat { // size is 0xC
	int data_0x00;
	void *data_0x04;
	SceIoStat *stat;
} SceVfsVopStat;

// call from sceVopGetstat, a1 size is 0xC(sceVopGetstat three args)
int vop_get_stat(SceVfsVopStat *vop_stat){

	const char *path;

	path = (const char *)*(uint32_t *)(vop_stat->data_0x04);

	if(strcmp(path, "host/.") == 0){
		vop_stat->stat->st_mode = SCE_S_IFDIR | SCE_S_IROTH | SCE_S_IRUSR | SCE_S_IWUSR;
		vop_stat->stat->st_attr = SCE_SO_IFDIR;
		vop_stat->stat->st_attr = 0;
		vop_stat->stat->st_size = 0xFFFFFFFFFFFFFFFFLL;

		vop_stat->stat->st_ctime.year        = 2020;
		vop_stat->stat->st_ctime.month       = 7;
		vop_stat->stat->st_ctime.day         = 5;
		vop_stat->stat->st_ctime.hour        = 1;
		vop_stat->stat->st_ctime.minute      = 33;
		vop_stat->stat->st_ctime.second      = 0;
		vop_stat->stat->st_ctime.microsecond = 0;

		memcpy(&vop_stat->stat->st_atime, &vop_stat->stat->st_ctime, sizeof(SceDateTime));
		memcpy(&vop_stat->stat->st_mtime, &vop_stat->stat->st_ctime, sizeof(SceDateTime));

		return 0;
	}

	return 0x80010086;
}

int sub_810021AE(void *a1){
	ksceDebugPrintf("sub_810021AE\n");
	sub_81002110((const char *)*(uint32_t *)(*(uint32_t *)(a1 + 4)));
	return 0x80010086;
}

int sub_8100219C(void *a1){
	ksceDebugPrintf("sub_8100219C\n");
	sub_81002110((const char *)*(uint32_t *)(*(uint32_t *)(a1 + 4)));
	return 0x80010086;
}

int sub_810020F8(void *a1){
	ksceDebugPrintf("sub_810020F8\n");
	return 0x80010086;
}

int sub_81002100(void *a1){
	ksceDebugPrintf("sub_81002100\n");
	return 0x80010086;
}

int sub_81002108(void *a1){
	ksceDebugPrintf("sub_81002108\n");
	return 0;
}

int sub_8100210C(void *a1){
	ksceDebugPrintf("sub_8100210C\n");
	return 0;
}

int sub_810020E8(void *a1){
	ksceDebugPrintf("sub_810020E8\n");
	return 0x80010086;
}

int sub_810020F0(void *a1){
	ksceDebugPrintf("sub_810020F0\n");
	return 0x80010086;
}

const SceVfsTable vfs_table = {
	.func00 = sub_81001DEC,
	.func04 = sub_81001DFE,
	.func08 = sub_81001E02,
	.func0C = NULL,
	.func10 = NULL,
	.func14 = NULL,
	.func18 = sub_81001E2E,
	.func1C = NULL,
	.func20 = sub_81001DE8,
	.func24 = NULL,
	.func28 = NULL,
	.func2C = sub_81001E32,
	.func30 = sub_8100222C
};

const SceVfsTable2 vfs_table2 = {
	.func00 = vop_open,
	.func04 = sub_81001E42,
	.func08 = vop_close,
	.func0C = vfs_part_init,
	.func10 = vop_read,
	.func14 = sub_81001E5A,
	.func18 = sub_810020C8,
	.func1C = NULL,
	.func20 = sub_81002208,
	.func24 = sub_810021F6,
	.func28 = sub_810021E4,
	.func2C = vop_dopen,
	.func30 = vop_dclose,
	.func34 = vop_dread,
	.func38 = vop_get_stat,
	.func3C = sub_810021AE,
	.func40 = sub_8100219C,
	.func44 = NULL,
	.func48 = sub_810020F8,
	.func4C = sub_81002100,
	.func50 = sub_81002108,
	.func54 = NULL,
	.func58 = NULL,
	.func5C = sub_8100210C,
	.func60 = sub_810020E8,
	.func64 = sub_810020F0,
	.func64 = NULL,
	.func68 = NULL,
	.func70 = NULL
};

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp){

	ksceVfsUnmount(&vfs_umount);

	ksceVfsDeleteVfs("bsod_dummy_host_fs", NULL);

	vfs_add.func_ptr1 = &vfs_table;
	vfs_add.device    = "faps_dummy_host_fs";
	vfs_add.data_0x08 = 0;
	vfs_add.data_0x0C = 0;
	vfs_add.data_0x10 = 0x10;
	vfs_add.func_ptr2 = &vfs_table2;
	vfs_add.data_0x18 = 0;

	ksceVfsAddVfs(&vfs_add);

	ksceVfsMount(&vfs_mount);

	return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize args, void *argp){
	return SCE_KERNEL_STOP_CANCEL;
}
