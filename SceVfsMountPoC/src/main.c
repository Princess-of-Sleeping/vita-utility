/*
 * SceVfs mount PoC
 * Copyright (C) 2020 Princess of Sleeping
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/sysmem.h>
#include <psp2kern/kernel/cpu.h>
#include <psp2kern/io/devctl.h>
#include <psp2kern/io/dirent.h>
#include <psp2kern/io/fcntl.h>
#include <string.h>
#include <stdio.h>
#include "vfs.h"
#include "sce_cpu.h"
#include "fs_mgr.h"
#include "item_mgr.h"
#include "module_kernel.h"
#include "sysroot_kblparam.h"
#include "etc.h"

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

SceUID heap_uid;
void *root_node;

SceUID (* sceKernelCreateUidObj)(SceClass *cls, const char *name, SceCreateUidObjOpt *opt, SceObjectBase **obj);

int (* sceKernelGetModuleList)(SceUID pid, int flags1, int flags2, SceUID *modids, size_t *num);
int (* sceKernelGetModuleInfo)(SceUID pid, SceUID modid, SceKernelModuleInfo *info);

#define GetExport(modname, lib_nid, func_nid, func) module_get_export_func(0x10005, modname, lib_nid, func_nid, (uintptr_t *)func)

int module_get_export_func(SceUID pid, const char *modname, uint32_t libnid, uint32_t funcnid, uintptr_t *func);

/**
 * @j 実行許可がない @ej
 * @e Operation is not permitted @ee
 */
#define SCE_ERROR_ERRNO_EPERM					-2147418111	/* 0x80010001 */

/**
 * @j ファイルがない @ej
 * @e Associated file or directory does not exist @ee
 */
#define SCE_ERROR_ERRNO_ENOENT					-2147418110	/* 0x80010002 */

int sub_81000008(void *a1){

	if(a1 == NULL)
		return 0x80010016;

	if(*(void **)(a1) == NULL)
		return 0x80010016;

	*(uint32_t *)(*(void **)(a1) + 0x68) = 0x80;
	*(uint32_t *)(*(void **)(a1) + 0xC0) = 0x10000;

	return 0;
}

int sub_81000020(void *a1){
	return 0;
}

int sub_81000024(void *a1){

	ksceDebugPrintf("sub_81000024\n");

	if(a1 == NULL)
		return 0x80010016;

	if(*(void **)(a1) == NULL)
		return 0x80010016;

	root_node = *(void **)(a1 + 8);

	DataPackForSystem_t *pDataPackForSystem;

	create_file_item(root_node, &pDataPackForSystem);

	pDataPackForSystem->data.dir_entry_root = NULL;
	pDataPackForSystem->data.dir_entry      = NULL;
	pDataPackForSystem->data.seek           = 0x0LL;
	pDataPackForSystem->data.size           = 0x0LL;

	init_module_kernel();
	init_sysroot();
	init_etc();

	*(uint32_t *)(*(void **)(a1 + 8) + 0x48) = *(uint32_t *)(*(uint32_t *)(a1) + 0xC4);
	*(uint32_t *)(*(void **)(a1 + 8) + 0x4C) = *(uint32_t *)(a1);
	*(uint32_t *)(*(void **)(a1 + 8) + 0x50) = 0;
	*(uint32_t *)(*(void **)(a1 + 8) + 0x60) = 0;
	*(uint32_t *)(*(void **)(a1 + 8) + 0x64) = 0;
	*(uint32_t *)(*(void **)(a1 + 8) + 0x74) = 1;
	*(uint32_t *)(*(void **)(a1 + 8) + 0x78) = 0x1002;
	*(uint32_t *)(*(void **)(a1 + 8) + 0x80) = 0;
	*(uint32_t *)(*(void **)(a1 + 8) + 0x84) = 0;
	*(uint32_t *)(*(void **)(a1 + 8) + 0x88) = 0;
	*(uint32_t *)(*(void **)(a1 + 8) + 0x8C) = 0;

	return 0;
}

int sub_81000000(){
	return 0;
}

int sub_81000004(){
	return 0;
}

int vfs_devctl(SceVfsDevctl *args){

	if(strcmp(args->dev, "host0:") != 0)
		return -1;

	if(args->cmd == 0x3001 && args->outlen == sizeof(SceIoDevInfo)){

		((SceIoDevInfo *)args->outdata)->max_size  = 0xFFFFFFFFFFFFFFFF;
		((SceIoDevInfo *)args->outdata)->free_size = 0x123456789ABCDEF0; // used space
		((SceIoDevInfo *)args->outdata)->cluster_size = 0;
		((SceIoDevInfo *)args->outdata)->unk = NULL;

		return 0;
	}

	return -1;
}

const SceVfsTable vfs_table_2 = {
	.func00 = sub_81000008,
	.func04 = sub_81000020,
	.func08 = sub_81000024,
	.func0C = NULL,
	.func10 = NULL,
	.func14 = NULL,
	.func18 = NULL,
	.func1C = NULL,
	.func20 = sub_81000000,
	.func24 = sub_81000004,
	.func28 = NULL,
	.func2C = vfs_devctl,
	.func30 = NULL  // PathElme
};

int vfs_open(SceVfsOpen *args){
/*
	ksceDebugPrintf("vfs_open\n");
	ksceDebugPrintf("path  : %s\n", args->path_info->path);
	ksceDebugPrintf("prev_node : 0x%X\n", args->node->prev_node);
	ksceDebugPrintf("node  : 0x%X\n", args->node);
	ksceDebugPrintf("flags : 0x%X\n", args->flags);
*/
	DataPack_t *pDataPack = getFileEntryByNode(args->node->prev_node);
	if(pDataPack == NULL)
		return -1;

	FileEntry *pDirEntry;
	int res;

	res = getFileEntry(args->path_info->path, &pDirEntry);
	if(res < 0)
		return SCE_ERROR_ERRNO_ENOENT;

	if((args->flags & ~pDirEntry->open_allow_flags) != 0){

		ksceDebugPrintf("Has flags that are not allowed in open\n");
		ksceDebugPrintf("flags          : 0x%08X\n", args->flags);
		ksceDebugPrintf("flags(illegal) : 0x%08X\n", (args->flags & ~pDirEntry->open_allow_flags));

		return SCE_ERROR_ERRNO_EPERM;
	}

	DataPackForSystem_t *pDataPackForSystem;

	create_file_item(args->node, &pDataPackForSystem);

	pDataPackForSystem->data.dir_entry_root = NULL;
	pDataPackForSystem->data.dir_entry      = pDirEntry;
	pDataPackForSystem->data.seek           = 0x0LL;
	pDataPackForSystem->data.size           = pDirEntry->size;

	if(pDirEntry->open_cb != NULL)
		pDirEntry->open_cb(&pDataPackForSystem->data, pDirEntry->args_for_stat);

	return 0;
}

int sub_81001E42(void *a1){
	ksceDebugPrintf("sub_81001E42\n");
	return 0x80010086;
}

int vfs_close(SceVfsClose *args){

	DataPack_t *pDataPack = getFileEntryByNode(args->node);
	if(pDataPack == NULL)
		return -1;

	return ksceKernelDeleteUid(pDataPack->uid);
}

int vfs_read(SceVfsRead *args){

	DataPack_t *pDataPack = getFileEntryByNode(args->node);
	if(pDataPack == NULL)
		return -1;

	if(pDataPack->dir_entry->read_cb == NULL)
		return -1;

	return pDataPack->dir_entry->read_cb(pDataPack, pDataPack->dir_entry->args_for_stat, args->data, args->size);
}

int vfs_write(SceVfsWrite *args){

	DataPack_t *pDataPack = getFileEntryByNode(args->node);
	if(pDataPack == NULL)
		return -1;

	if(pDataPack->dir_entry->write_cb == NULL)
		return -1;

	return pDataPack->dir_entry->write_cb(pDataPack, pDataPack->dir_entry->args_for_stat, args->data, args->size);
}

int sub_8100006C(){
	ksceDebugPrintf("sub_8100006C\n");
	return 0;
}

int sub_81000070(){
	ksceDebugPrintf("sub_81000070\n");
	return 0;
}

int vfs_dopen(SceVfsDopen *args){
/*
	ksceDebugPrintf("dopen\n");
	ksceDebugPrintf("%s\n", args->opts->path);
	ksceDebugPrintf("node : 0x%X\n", args->node);
*/
	if(args->node == root_node){
		DataPack_t *pDataPack = getFileEntryByNode(args->node);
		if(pDataPack == NULL)
			return 0x80000000;

		if(pDataPack->dir_entry_root == NULL)
			return 0x80000001;

		pDataPack->dir_entry = pDataPack->dir_entry_root;
	}else{
		FileEntry *pDirEntry;

		int res;

		res = getDirEntry(args->opts->path, &pDirEntry);
		if(res < 0)
			return -1;

		DataPackForSystem_t *pDataPackForSystem;

		create_file_item(args->node, &pDataPackForSystem);

		pDataPackForSystem->data.dir_entry_root = pDirEntry->next_dir;
		pDataPackForSystem->data.dir_entry      = pDirEntry->next_dir;
		pDataPackForSystem->data.seek           = 0x0LL;
		pDataPackForSystem->data.size           = pDirEntry->size;

		if(pDirEntry->dopen_cb != NULL){
			pDirEntry->dopen_cb(&pDataPackForSystem->data, NULL);
			pDirEntry->next_dir = pDataPackForSystem->data.dir_entry_root;
			pDataPackForSystem->data.dir_entry = pDirEntry->next_dir;
		}
	}

	return 0;
}

int vfs_dclose(SceVfsDclose *args){
	// ksceDebugPrintf("dclose\n");

	if(args->node == root_node){
		return 0;
	}

	DataPack_t *pDataPack = getFileEntryByNode(args->node);
	if(pDataPack == NULL)
		return -1;

	return ksceKernelDeleteUid(pDataPack->uid);
}

int vfs_dread(SceVfsDread *args){
/*
	ksceDebugPrintf("dread\n");
	ksceDebugPrintf("vfs_node : 0x%X\n", args->vfs_node);
*/
	DataPack_t *pDataPack = getFileEntryByNode(args->vfs_node);
	if(pDataPack == NULL)
		return -1;

	int res = (pDataPack->dir_entry == NULL) ? 0 : 1;

	if(pDataPack->dir_entry == NULL)
		return res;

	if(pDataPack->dir_entry->get_io_stat_cb == NULL){

		if(pDataPack->dir_entry->stat == NULL){
			return -1;
		}

		memcpy(&args->dir->d_stat, pDataPack->dir_entry->stat, sizeof(SceIoStat));
		args->dir->d_stat.st_size = pDataPack->dir_entry->size;
	}else{
		pDataPack->dir_entry->get_io_stat_cb(pDataPack->dir_entry, pDataPack->dir_entry->args_for_stat, &args->dir->d_stat);
	}

	snprintf(args->dir->d_name, 255, "%s", pDataPack->dir_entry->name);
	args->dir->d_private = NULL;
	args->dir->dummy = 0;

	pDataPack->dir_entry = pDataPack->dir_entry->next;

	return res;
}

extern const SceIoStat stat_file_tpl;

int vfs_get_stat(SceVfsStat *args){
	// ksceDebugPrintf("sub_get_stat\n");

	if(args->node == root_node){
		memcpy(args->stat, &stat_file_tpl, sizeof(SceIoStat));
		args->stat->st_size = 0x100000LL;
		return 0;
	}

	DataPack_t *pDataPack = getFileEntryByNode(args->node->prev_node);
	if(pDataPack == NULL)
		return -1;

	const FileEntry *dir_entry = pDataPack->dir_entry_root;
	while(dir_entry != NULL){
		if(strcmp(args->opts->path, dir_entry->name) == 0){
			if(dir_entry->get_io_stat_cb == NULL){
				memcpy(args->stat, dir_entry->stat, sizeof(SceIoStat));
				args->stat->st_size = dir_entry->size;
			}else{
				dir_entry->get_io_stat_cb(dir_entry, dir_entry->args_for_stat, args->stat);
			}

			return 0;
		}
		dir_entry = dir_entry->next;
	}

	return -1;
}

int vfs_part_init(SceVfsPartInit *a1){

	int res;
	SceVfsNode *pNode = NULL;
/*
	ksceDebugPrintf("vfs_part_init\n");
	ksceDebugPrintf("%s\n", *(char **)(a1->opt));
	ksceDebugPrintf("0x%X\n", *(int *)(a1->opt + 4));

	if(*(int *)(a1->opt + 4) != 3) // file name len check?
		return 0x80010016;
*/
	if(a1->node->mount == NULL)
		return 0x80010016;

	res = ksceVfsGetNewNode(a1->node->mount, *(uint32_t *)(*(uint32_t *)(a1->node->mount + 0x5C) + 0x14), 0, &pNode);
	if(res < 0){
		ksceDebugPrintf("sceVfsGetNewNode : 0x%X\n", res);
		return res;
	}

	if(pNode == NULL)
		return 0x80010016;

	// ksceDebugPrintf("NewNode : 0x%X\n", pNode);

	ksceVfsNodeWaitEventFlag(pNode);

	pNode->dev_info  = NULL;
	pNode->mount     = a1->node->mount;
	pNode->prev_node = a1->node;
	pNode->unk_74    = 1;
	pNode->unk_78    = 0x2010;

	if(getDirEntry(*(char **)(a1->opt), NULL) == 0)
		pNode->unk_78 = SCE_S_IFDIR | SCE_S_IWUSR | SCE_S_IRUSR;

	*a1->new_node = pNode;

	return 0;
}

int vfs_get_stat_by_fd(SceVfsStatByFd *args){
	// ksceDebugPrintf("vfs_get_stat_by_fd\n");

	DataPack_t *pDataPack = getFileEntryByNode(args->node);
	if(pDataPack == NULL)
		return -1;

	if(pDataPack->dir_entry->get_io_stat_cb == NULL){

		memcpy(args->stat, pDataPack->dir_entry->stat, sizeof(SceIoStat));

		args->stat->st_size = pDataPack->dir_entry->size;
	}else{
		pDataPack->dir_entry->get_io_stat_cb(pDataPack->dir_entry, pDataPack->dir_entry->args_for_stat, args->stat);
	}

	return -1;
}

int vfs_part_deinit(void *a1){
	ksceDebugPrintf("sub_part_deinit\n");
	return 0x80010086;
}

int vfs_sync(void *a1){
	ksceDebugPrintf("sub_sync\n");
	return 0;
}

const SceVfsTable2 vfs_table2_2 = {
	.func00 = vfs_open,
	.func04 = sub_81001E42,
	.func08 = vfs_close,
	.func0C = vfs_part_init,
	.func10 = vfs_read,
	.func14 = vfs_write,
	.func18 = NULL,
	.func1C = sub_8100006C, // Ioctl
	.func20 = vfs_part_deinit,
	.func24 = NULL,
	.func28 = NULL,
	.func2C = vfs_dopen,
	.func30 = vfs_dclose,
	.func34 = vfs_dread,
	.func38 = vfs_get_stat,
	.func3C = NULL,
	.func40 = NULL,
	.func44 = NULL,
	.func48 = NULL,
	.func4C = NULL,
	.func50 = sub_81000070,
	.func54 = NULL,
	.func58 = NULL,
	.func5C = vfs_sync,
	.func60 = vfs_get_stat_by_fd,
	.func64 = NULL,
	.func64 = NULL,
	.func68 = NULL,
	.func70 = NULL
};

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp){

	if(GetExport("SceKernelModulemgr", 0xC445FA63, 0xD269F915, &sceKernelGetModuleInfo) < 0)
	if(GetExport("SceKernelModulemgr", 0x92C9FFC2, 0xDAA90093, &sceKernelGetModuleInfo) < 0)
		return SCE_KERNEL_START_FAILED;

	if(GetExport("SceKernelModulemgr", 0xC445FA63, 0x97CF7B4E, &sceKernelGetModuleList) < 0)
	if(GetExport("SceKernelModulemgr", 0x92C9FFC2, 0xB72C75A4, &sceKernelGetModuleList) < 0)
		return SCE_KERNEL_START_FAILED;

	if(GetExport("SceSysmem", 0x63A519E5, 0xDF0288D7, &sceKernelCreateUidObj) < 0)
	if(GetExport("SceSysmem", 0x02451F0F, 0xFB6390CE, &sceKernelCreateUidObj) < 0)
		return SCE_KERNEL_START_FAILED;

	init_itemmgr();

	heap_uid = ksceKernelCreateHeap("FapsVfsHost0", 0x40000, NULL);

	init_l2_cache_reg();

	ksceVfsUnmount(&vfs_umount);

	ksceVfsDeleteVfs("bsod_dummy_host_fs", NULL);

	vfs_add.func_ptr1 = &vfs_table_2;
	vfs_add.device    = "faps_dummy_host_fs";
	vfs_add.data_0x08 = 0x11;
	vfs_add.data_0x0C = 0;
	vfs_add.data_0x10 = 0x10;
	vfs_add.func_ptr2 = &vfs_table2_2;
	vfs_add.data_0x18 = 0;

	ksceVfsAddVfs(&vfs_add);

	ksceVfsMount(&vfs_mount);

	return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize args, void *argp){
	return SCE_KERNEL_STOP_CANCEL;
}
