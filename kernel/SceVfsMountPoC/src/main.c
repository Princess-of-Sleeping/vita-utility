/*
 * SceVfs mount PoC
 * Copyright (C) 2020 Princess of Sleeping
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/sysmem.h>
#include <psp2kern/kernel/sysclib.h>
#include <psp2kern/kernel/proc_event.h>
#include <psp2kern/io/devctl.h>
#include <psp2kern/io/dirent.h>
#include <psp2kern/io/fcntl.h>
#include "vfs.h"
#include "fs_tool.h"
#include "dmass_process.h"

char *strrchr(const char *s, int c);

#define SCE_CST_MODE        0x0001
#define SCE_CST_SIZE        0x0004
#define SCE_CST_CT          0x0008
#define SCE_CST_AT          0x0010
#define SCE_CST_MT          0x0020

#define FAPS_DMASS_DEVICE_NAME "vsd0:"

SceVfsAddParam vfs_add;
// extern SceUID test_heap_uid;

int vfs_devctl(SceVfsDevctl *args){

	if(strcmp(args->dev, FAPS_DMASS_DEVICE_NAME) != 0)
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

int vfs_mount_func(void *argp){
	return 0;
}

int vfs_umount_func(void *argp){
	return 0;
}

int vfs_set_root(SceVfsopSetRootArgs *argp){

	ksceDebugPrintf("%s\n", __FUNCTION__);

	FapsDmassFs *pDmassFs;

	if(argp == NULL)
		return 0x80010016;

	if(argp->pMount == NULL)
		return 0x80010016;

	pDmassFs = create_dmass_dir_info("md");
	if(pDmassFs == NULL){
		return -1;
	}

	pDmassFs->pNode = argp->pNode;

	set_fs_root(pDmassFs);

	argp->pNode->core.dev_info  = pDmassFs;
	argp->pNode->core.mount     = argp->pMount;
	argp->pNode->core.prev_node = NULL;
	argp->pNode->core.unk_60    = 0;
	argp->pNode->core.unk_64    = 0;
	argp->pNode->core.unk_74    = 1;
	argp->pNode->core.unk_78    = 0x1002;
	argp->pNode->core.st_size   = 0LL;
	argp->pNode->core.st_attr   = 0;
	argp->pNode->core.unk_8C    = 0;

	return 0;
}

int vfs_init(void *a1){
	return 0;
}

int vfs_deinit(void *a1){
	return 0;
}

int vop_open(SceVfsOpen *argp){
	ksceDebugPrintf("%s\n", __FUNCTION__);
	ksceDebugPrintf("\tnode : %p\n", argp->node);
	ksceDebugPrintf("\tname : %s (%d)\n", argp->file_info->name, argp->file_info->name_len);
	ksceDebugPrintf("\tpath : %s\n", argp->file_info->path);

	FapsDmassFs *pEntry;

	pEntry = (FapsDmassFs *)argp->node->core.dev_info;
	if(pEntry == NULL)
		return -1;

	FapsDmassFsX *pObject = create_dmass_ctl_data(pEntry, (int)(argp->pVfsFileObject));
	if(pObject == NULL)
		return -1;

	if((argp->pVfsFileObject->flags & SCE_O_APPEND) != 0){
		argp->pVfsFileObject->offset = pEntry->stat.st_size;
	}

	return 0;
}

int vop_create(SceVopCreateArgs *argp){
	ksceDebugPrintf("%s\n", __FUNCTION__);
	ksceDebugPrintf("\tnode     : %p\n", argp->pNode);
	ksceDebugPrintf("\tname     : %s (%d)\n", argp->file_info->name, argp->file_info->name_len);
	ksceDebugPrintf("\tpath     : %s\n", argp->file_info->path);
	ksceDebugPrintf("\tflags    : 0x%X\n", argp->flags);
	ksceDebugPrintf("\tmode     : 0%o\n", argp->mode);

	int res;
	SceVfsNode *pNode;
	FapsDmassFs *pDmassFs;

	res = get_fs_entry(argp->file_info->path, NULL);
	if(res >= 0)
		return 0x80010011; // Directory already exist

	pDmassFs = create_dmass_file_info(argp->file_info->name);
	if(pDmassFs == NULL){
		return -1;
	}

	pDmassFs->next = ((FapsDmassFs *)argp->pNode->core.dev_info)->next_dir;
	((FapsDmassFs *)argp->pNode->core.dev_info)->next_dir = pDmassFs;

    res = ksceVfsGetNewNode(argp->pNode->core.mount, argp->pNode->core.mount->pVfsAddParam->pVopTable, 0, &pNode);
	if(res < 0){
		return res;
	}

    res = ksceVfsNodeWaitEventFlag(pNode);
    if(res < 0){
    	ksceVfsFreeVnode(pNode);
		return res;
	}

	ksceDebugPrintf("\tnew node : %p\n", pNode);

	pDmassFs->pNode = pNode;

    pNode->core.mount           = argp->pNode->core.mount;
    pNode->core.st_size         = 0LL;
    pNode->core.dev_info        = pDmassFs;
    pNode->core.prev_node       = argp->pNode;
    pNode->core.unk_74          = 1;
	pNode->core.unk_78          = SCE_S_IXUSR | SCE_S_IWUSR | SCE_S_IRUSR;
	pNode->core.st_attr         = SCE_SO_IXOTH | SCE_SO_IWOTH | SCE_SO_IROTH;
    pNode->core.unk_8C          = 0;
    pNode->core.some_counter_58 = 1;

	*(argp->ppNewNode) = pNode;

	return 0;
}

int vop_close(SceVfsClose *argp){
	ksceDebugPrintf("%s\n", __FUNCTION__);
	ksceDebugPrintf("\tnode : %p\n", argp->node);

	int res;

	FapsDmassFs *pEntry = ((FapsDmassFs *)argp->node->core.dev_info);
	if(pEntry == NULL)
		return -1;

	FapsDmassFsX *pObject = search_dmass_ctl_data(pEntry, (int)(argp->pVfsFileObject));
	if(pObject == NULL)
		return -1;

	res = delete_dmass_ctl_data(pEntry, (int)(argp->pVfsFileObject));
	if(res < 0)
		return res;

	return 0;
}

// vop_lookup
int vop_part_init(SceVfsPartInit *argp){
	ksceDebugPrintf("%s\n", __FUNCTION__);

/*
 * 0x40 : rename
 */
	ksceDebugPrintf("\tflags=0x%X path=\"%s\" name=\"%s\" %s\n", argp->flags, argp->path_info->path, argp->path_info->name, argp->path_info->msg);

	if(argp->path_info != NULL && 0){
		ksceDebugPrintf("\topt 0x0=%s\n", argp->path_info->name);
		ksceDebugPrintf("\topt 0x4=%s\n", argp->path_info->msg);
		ksceDebugPrintf("\topt 0x8=%s\n", argp->path_info->path);
		ksceDebugPrintf("\topt 0xC=%s\n", argp->path_info->name2);
	}

	int res;
	SceVfsNode *pNode = NULL;

	res = check_name(argp->path_info->name);
	if(res == 0x80020005)
		return 0x80010002;

	if(res < 0)
		return res;

	if(argp->node->core.mount == NULL)
		return 0x80010016;

	FapsDmassFs *pEntry;

	res = get_fs_entry(argp->path_info->path, &pEntry);
	if(res < 0){
		ksceDebugPrintf("\tget_fs_entry : 0x%X\n", res);
		return res;
	}

	res = ksceVfsGetNewNode(argp->node->core.mount, argp->node->core.mount->pVfsAddParam->pVopTable, 0, &pNode);
	if(res < 0){
		ksceDebugPrintf("\tsceVfsGetNewNode : 0x%X\n", res);
		return res;
	}

	res = ksceVfsNodeWaitEventFlag(pNode);
	if(res < 0){
		ksceVfsFreeVnode(pNode);
		return res;
	}

	pEntry->pNode = pNode;

	pNode->core.dev_info  = pEntry;
	pNode->core.mount     = argp->node->core.mount;
	pNode->core.prev_node = argp->node;
	pNode->core.unk_74    = 1;
	pNode->core.st_attr   = pEntry->stat.st_attr;
	pNode->core.st_size   = pEntry->stat.st_size;
    pNode->core.unk_8C    = 0;
    pNode->core.some_counter_58 = 1;

	if(SCE_S_ISDIR(pEntry->stat.st_mode)){
		pNode->core.unk_78 = SCE_S_IWUSR;
	}else{
		pNode->core.unk_78 = SCE_S_IXUSR;
	}

	*(argp->new_node) = pNode;

	ksceDebugPrintf("\t%s out\n", __FUNCTION__);

	return 0;
}

#define FAPS_DMASS_BLOCK_SIZE (0x200)
#define FAPS_DMASS_BLOCK_ALIGN(__value__) (((__value__) + (FAPS_DMASS_BLOCK_SIZE - 1)) & ~(FAPS_DMASS_BLOCK_SIZE - 1))

SceSSize vop_read(SceVfsRead *argp){

	const void *lr;
	asm volatile("mov %0, lr\n":"=r"(lr));

	FapsDmassFs *pEntry = ((FapsDmassFs *)argp->node->core.dev_info);

	ksceDebugPrintf("%s(%p)\n", __FUNCTION__, lr);
	ksceDebugPrintf("\tdata=%p nbyte=0x%08X\n", argp->data, argp->nbyte);
	ksceDebugPrintf("\tcurrent size:offset=0x%08llX:0x%08llX\n", argp->node->core.st_size, argp->pVfsFileObject->offset);

	if(pEntry == NULL)
		return -1;

	FapsDmassFsX *pObject = search_dmass_ctl_data(pEntry, (int)(argp->pVfsFileObject));
	if(pObject == NULL)
		return -1;

	if(pEntry->stat.st_size > 0xFFFFFFFF || argp->pVfsFileObject->offset > 0xFFFFFFFF)
		return -1;

	if(argp->node != argp->pVfsFileObject->node)
		return -3;

	SceSize r_size = argp->nbyte;

	if(r_size == 0xdeadbeef)
		return -2;

	SceOff remain_size = pEntry->stat.st_size - argp->pVfsFileObject->offset;
	if(remain_size > 0xFFFFFFFF || remain_size < 0LL)
		return -1;

	ksceDebugPrintf("\tremain_size=0x%llX\n", remain_size);

	if(r_size >= (SceSize)remain_size)
		r_size = (SceSize)remain_size;

	memcpy(argp->data, pEntry->data + argp->pVfsFileObject->offset, r_size);

	ksceDebugPrintf("\tr_size=0x%X\n", r_size);

	argp->pVfsFileObject->offset += r_size;

	return r_size;
}

SceSSize vop_write(SceVfsWrite *argp){

	const void *lr;
	asm volatile("mov %0, lr\n":"=r"(lr));

	FapsDmassFs *pEntry = ((FapsDmassFs *)argp->node->core.dev_info);

	ksceDebugPrintf("%s(%p)\n", __FUNCTION__, lr);
	ksceDebugPrintf("\tdata=%p nbyte=0x%08X\n", argp->data, argp->nbyte);
	ksceDebugPrintf("\tcurrent size  =0x%llX\n", argp->node->core.st_size);
	ksceDebugPrintf("\tcurrent offset=0x%llX\n", argp->pVfsFileObject->offset);

	if(pEntry == NULL)
		return -1;

	FapsDmassFsX *pObject = search_dmass_ctl_data(pEntry, (int)(argp->pVfsFileObject));
	if(pObject == NULL)
		return -1;

	if(argp->node->core.st_size > 0xFFFFFFFF || argp->pVfsFileObject->offset > 0xFFFFFFFF)
		return -1;

	if(argp->node != argp->pVfsFileObject->node)
		return -3;

	SceSize w_size = argp->nbyte;

	SceOff size_align = FAPS_DMASS_BLOCK_ALIGN(pEntry->stat.st_size);
	if(FAPS_DMASS_BLOCK_ALIGN(pEntry->stat.st_size + w_size) > size_align){

		ksceDebugPrintf("\tGrow data\n");

		void *temp_data = dmass_malloc(FAPS_DMASS_BLOCK_ALIGN(pEntry->stat.st_size + w_size));
		if(temp_data == NULL)
			return -2;

		memcpy(temp_data, pEntry->data, (size_t)pEntry->stat.st_size);

		dmass_free(pEntry->data);
		pEntry->data = temp_data;
	}

	memcpy(pEntry->data + argp->pVfsFileObject->offset, argp->data, w_size);

	argp->pVfsFileObject->offset += w_size;

	if(argp->pVfsFileObject->offset >= pEntry->stat.st_size){
		pEntry->stat.st_size     = argp->pVfsFileObject->offset;
		argp->node->core.st_size = argp->pVfsFileObject->offset;
	}

	return w_size;
}

SceOff vop_lseek(SceVopLseekArgs *argp){
	ksceDebugPrintf("%s\n", __FUNCTION__);
	ksceDebugPrintf("\toffset=0x%llX where=%d\n", argp->offset, argp->where);

	SceOff offset;

	if(argp->where == SCE_SEEK_SET){
		offset = argp->offset;
	}else if(argp->where == SCE_SEEK_CUR){
		offset = argp->pVfsFileObject->offset + argp->offset;
	}else if(argp->where == SCE_SEEK_END){
		offset = argp->node->core.st_size + argp->offset;
	}else{
		return 0xFFFFFFFF80010016;
	}

	if(offset > argp->node->core.st_size || offset < 0LL)
		return 0xFFFFFFFF80010016;

	argp->pVfsFileObject->offset = offset;

	return offset;
}

int vop_ioctl(SceVopIoctlArgs *argp){
	return 0x80010030;
}

int vop_remove(SceVfsPartDeinit *argp){
	ksceDebugPrintf("%s\n", __FUNCTION__);
	ksceDebugPrintf("\tname=%s\n", argp->file_info->name);
	ksceDebugPrintf("\t    =%d\n", argp->file_info->name_len);
	ksceDebugPrintf("\tpath=%s\n", argp->file_info->path);

	int res;
	FapsDmassFs *pEntry;

	res = get_fs_entry(argp->file_info->path, &pEntry);
	if(res < 0)
		return 0x80010002;

	if(!SCE_S_ISREG(pEntry->stat.st_mode))
		return -1; // Not file

	FapsDmassFs **ppFsInfo = &(((FapsDmassFs *)argp->node->core.dev_info)->next_dir);
	while(*ppFsInfo != NULL){
		if(pEntry == *ppFsInfo){
			*ppFsInfo = (*ppFsInfo)->next;

			res = delete_dmass_info(pEntry);
			pEntry = NULL;

			return res;
		}

		ppFsInfo = &(*ppFsInfo)->next;
	}

	return 0x80010002;
}

int vop_mkdir(SceVopMkdirArgs *argp){
	ksceDebugPrintf("%s\n", __FUNCTION__);
	ksceDebugPrintf("\tnode=%p mode=0%o /%s <- /%s\n", argp->node, argp->mode, ((FapsDmassFs *)argp->node->core.dev_info)->name, argp->file_info->name);

	int res;
	SceVfsNode *pNode = NULL;
	FapsDmassFs *pDmassFs;

	if(!SCE_S_ISDIR(((FapsDmassFs *)argp->node->core.dev_info)->stat.st_mode))
		return 0x80010014; // Not directory

	res = get_fs_entry(argp->file_info->path, NULL);
	if(res >= 0)
		return 0x80010011; // Directory already exist

	pDmassFs = create_dmass_dir_info(argp->file_info->name);
	if(pDmassFs == NULL){
		return -1;
	}

	pDmassFs->next     = ((FapsDmassFs *)argp->node->core.dev_info)->next_dir;
	((FapsDmassFs *)argp->node->core.dev_info)->next_dir = pDmassFs;

    res = ksceVfsGetNewNode(argp->node->core.mount, argp->node->core.mount->pVfsAddParam->pVopTable, 0, &pNode);
	if(res < 0){
		return res;
	}

    res = ksceVfsNodeWaitEventFlag(pNode);
    if(res < 0){
    	ksceVfsFreeVnode(pNode);
		return res;
	}

	pDmassFs->pNode = pNode;

    pNode->core.mount           = argp->node->core.mount;
    pNode->core.st_size         = 0LL;
    pNode->core.dev_info        = pDmassFs;
    pNode->core.prev_node       = argp->node;
    pNode->core.unk_74          = 1;
    pNode->core.unk_78          = SCE_S_IWUSR;
	pNode->core.st_attr         = SCE_SO_IXOTH | SCE_SO_IWOTH | SCE_SO_IROTH | SCE_SO_IFDIR;
    pNode->core.unk_8C          = 0;
    pNode->core.some_counter_58 = 1;

	*(argp->pNewNode) = pNode;

	return 0;
}

int vop_rmdir(SceVopRmdirArgs *argp){
	ksceDebugPrintf("%s\n", __FUNCTION__);
	ksceDebugPrintf("\tname=%s\n", argp->file_info->name);
	ksceDebugPrintf("\tpath=%s\n", argp->file_info->path);

	int res;
	FapsDmassFs *pEntry;

	res = get_fs_entry(argp->file_info->path, &pEntry);
	if(res < 0)
		return res;

	if(pEntry->next_dir != NULL)
		return 0x80010011; // Directory is have more entries

	if(!SCE_S_ISDIR(pEntry->stat.st_mode))
		return 0x80010014; // Not directory

	FapsDmassFs **ppFsInfo = &(((FapsDmassFs *)argp->node->core.dev_info)->next_dir);
	while(*ppFsInfo != NULL){
		if(pEntry == *ppFsInfo){
			*ppFsInfo = (*ppFsInfo)->next;

			res = delete_dmass_info(pEntry);
			pEntry = NULL;

			return res;
		}

		ppFsInfo = &(*ppFsInfo)->next;
	}

	return 0x80010002;
}

int vop_dopen(SceVfsDopen *argp){
	ksceDebugPrintf("%s\n", __FUNCTION__);
	ksceDebugPrintf("\t%s (%d)\n", argp->file_info->name, argp->file_info->name_len);
	ksceDebugPrintf("\t%s\n", argp->file_info->path);
	ksceDebugPrintf("\tdev_info=%p\n", argp->node->core.dev_info);

	FapsDmassFs *pEntry = ((FapsDmassFs *)argp->node->core.dev_info);
	if(pEntry == NULL)
		return -1;

	FapsDmassFsX *pObject = create_dmass_ctl_data(pEntry, (int)(argp->pVfsFileObject));
	if(pObject == NULL)
		return 0x80010016;

	pObject->current = pEntry->next_dir;

	return 0;
}

int vop_dclose(SceVfsDclose *argp){
	ksceDebugPrintf("%s\n", __FUNCTION__);

	int res;

	FapsDmassFs *pEntry = ((FapsDmassFs *)argp->node->core.dev_info);
	if(pEntry == NULL)
		return -1;

	FapsDmassFsX *pObject = search_dmass_ctl_data(pEntry, (int)(argp->pVfsFileObject));
	if(pObject == NULL)
		return 0x80010016;

	res = delete_dmass_ctl_data(pEntry, (int)(argp->pVfsFileObject));
	if(res < 0)
		return 0x80010016;

	return 0;
}

int vop_dread(SceVfsDread *argp){
	ksceDebugPrintf("%s\n", __FUNCTION__);

	FapsDmassFs *pEntry = ((FapsDmassFs *)argp->node->core.dev_info);
	if(pEntry == NULL)
		return -1;

	FapsDmassFsX *pObject = search_dmass_ctl_data(pEntry, (int)(argp->pVfsFileObject));
	if(pObject == NULL)
		return 0x80010016;

	if(pObject->current == NULL)
		return 0;

	argp->dir->d_name[0xFF] = 0;
	strncpy(argp->dir->d_name, pObject->current->name, 0xFF);
	memcpy(&(argp->dir->d_stat), &(pObject->current->stat), sizeof(SceIoStat));

	pObject->current = pObject->current->next;

	return 1;
}

int vop_get_stat(SceVfsStat *argp){
	ksceDebugPrintf("%s\n", __FUNCTION__);

	int res;
	FapsDmassFs *pEntry;

	res = get_fs_entry(argp->file_info->path, &pEntry);
	if(res < 0)
		return res;

	memcpy(argp->stat, &(pEntry->stat), sizeof(*(argp->stat)));

	return 0;
}

int vop_chstat(SceVopChstatArgs *argp){
	ksceDebugPrintf("%s\n", __FUNCTION__);

	int res;
	FapsDmassFs *pEntry;

	res = get_fs_entry(argp->file_info->path, &pEntry);
	if(res < 0)
		return res;

	if((argp->bit & SCE_CST_MODE) != 0){

		if((pEntry->stat.st_mode & SCE_S_IFMT) != (argp->stat->st_mode & SCE_S_IFMT))
			return -1;

		pEntry->stat.st_mode = argp->stat->st_mode;
	}

	if((argp->bit & SCE_CST_SIZE) != 0){
		if(argp->stat->st_size < 0LL)
			return -1;

		if(argp->stat->st_size == 0LL){
			dmass_free(pEntry->data);
			pEntry->data = NULL;
		}else{
			if(argp->stat->st_size > 0xFFFFFFFF)
				return -1;

			void *temp_data = dmass_malloc((SceSize)argp->stat->st_size);
			if(temp_data == NULL)
				return -2;

			memcpy(temp_data, pEntry->data, (size_t)pEntry->stat.st_size);

			dmass_free(pEntry->data);
			pEntry->data = temp_data;
		}

		pEntry->stat.st_size     = argp->stat->st_size;
		argp->node->core.st_size = argp->stat->st_size;
	}

	if((argp->bit & SCE_CST_CT) != 0)
		memcpy(&(pEntry->stat.st_ctime), &(argp->stat->st_ctime), sizeof(SceDateTime));

	if((argp->bit & SCE_CST_AT) != 0)
		memcpy(&(pEntry->stat.st_atime), &(argp->stat->st_atime), sizeof(SceDateTime));

	if((argp->bit & SCE_CST_MT) != 0)
		memcpy(&(pEntry->stat.st_mtime), &(argp->stat->st_mtime), sizeof(SceDateTime));

	return 0;
}

int vop_rename(SceVopRenameArgs *argp){
	ksceDebugPrintf("%s\n", __FUNCTION__);

	if(argp->node->core.mount != argp->node2->core.mount)
		return 0x80010001;

	ksceDebugPrintf("\told name %s\n", argp->file_info_old->name);
	ksceDebugPrintf("\told      %d\n", argp->file_info_old->name_len);
	ksceDebugPrintf("\told path %s\n", argp->file_info_old->path);

	ksceDebugPrintf("\tnew name %s\n", argp->file_info_new->name);
	ksceDebugPrintf("\tnew      %d\n", argp->file_info_new->name_len);
	ksceDebugPrintf("\tnew path %s\n", argp->file_info_new->path);

	ksceDebugPrintf("\tnode  %p\n", argp->node);
	ksceDebugPrintf("\tnode2 %p\n", argp->node2);
	ksceDebugPrintf("\tnode3 %p\n", argp->node3);

	int res;
	FapsDmassFs *pEntryOld;

	res = get_fs_entry(argp->file_info_old->path, &pEntryOld);
	if(res < 0)
		return res;

	FapsDmassFs **ppFsInfo = &(((FapsDmassFs *)argp->node->core.dev_info)->next_dir);
	while(*ppFsInfo != NULL){
		if(pEntryOld == *ppFsInfo){

			SceVfsNode *pNode = NULL;

    		res = ksceVfsGetNewNode(argp->node->core.mount, argp->node->core.mount->pVfsAddParam->pVopTable, 0, &pNode);
		    if(res < 0)
				return res;

		    res = ksceVfsNodeWaitEventFlag(pNode);
		    if(res < 0){
    			ksceVfsFreeVnode(pNode);
				return res;
			}

			*ppFsInfo = (*ppFsInfo)->next;

			pEntryOld->next = ((FapsDmassFs *)argp->node3->core.dev_info)->next_dir;
			((FapsDmassFs *)argp->node3->core.dev_info)->next_dir = pEntryOld;

			dmass_free(pEntryOld->name);
			pEntryOld->name = name_cpy(argp->file_info_new->name);

			ksceDebugPrintf("%p\n", pEntryOld->pNode);

		    pNode->core.mount           = pEntryOld->pNode->core.mount;
		    pNode->core.st_size         = pEntryOld->pNode->core.st_size;
		    pNode->core.dev_info        = pEntryOld;
		    pNode->core.prev_node       = pEntryOld->pNode;
		    pNode->core.unk_74          = 1;
		    pNode->core.unk_78          = pEntryOld->pNode->core.unk_78;
			pNode->core.st_attr         = pEntryOld->pNode->core.st_attr;
		    pNode->core.unk_8C          = pEntryOld->pNode->core.unk_8C;
		    pNode->core.some_counter_58 = 1;

			pEntryOld->pNode = pNode;

			*(argp->ppNewNode) = pNode;

			return 0;
		}

		ppFsInfo = &(*ppFsInfo)->next;
	}

	return 0x80010002;
}

SceSSize vop_pread(SceVopPreadArgs *argp){
	ksceDebugPrintf("%s\n", __FUNCTION__);

	FapsDmassFs *pEntry = ((FapsDmassFs *)argp->node->core.dev_info);
	if(pEntry == NULL)
		return -1;

	FapsDmassFsX *pObject = search_dmass_ctl_data(pEntry, (int)(argp->pVfsFileObject));
	if(pObject == NULL)
		return -1;

	if(argp->node->core.st_size > 0xFFFFFFFF || argp->offset > 0xFFFFFFFF)
		return -1;

	if(argp->offset > argp->node->core.st_size)
		return -1;

	if(argp->node != argp->pVfsFileObject->node)
		return -3;

	if(argp->offset >= argp->node->core.st_size)
		return 0;

	SceSize r_size = argp->nbyte;

	SceOff remain_size = argp->node->core.st_size - argp->offset;
	if(r_size >= (SceSize)remain_size)
		r_size = (SceSize)remain_size;

	memcpy(argp->data, pEntry->data + argp->offset, r_size);

	return r_size;
}

SceSSize vop_pwrite(SceVopPwriteArgs *argp){

	const void *lr;
	asm volatile("mov %0, lr\n":"=r"(lr));

	ksceDebugPrintf("%s(%p)\n", __FUNCTION__, lr);
	ksceDebugPrintf("\tdata=%p nbyte=0x%08X offset=0x%08llX\n", argp->data, argp->nbyte, argp->offset);
	ksceDebugPrintf("\tcurrent size  =0x%llX\n", argp->node->core.st_size);
	ksceDebugPrintf("\tcurrent offset=0x%llX\n", argp->pVfsFileObject->offset);

	FapsDmassFs *pEntry = ((FapsDmassFs *)argp->node->core.dev_info);
	if(pEntry == NULL)
		return -1;

	FapsDmassFsX *pObject = search_dmass_ctl_data(pEntry, (int)(argp->pVfsFileObject));
	if(pObject == NULL)
		return -1;

	if(argp->node->core.st_size > 0xFFFFFFFF || argp->offset > 0xFFFFFFFF)
		return -1;

	if(argp->offset > argp->node->core.st_size)
		return -1;

	if(argp->node != argp->pVfsFileObject->node)
		return -3;

	SceSize w_size = argp->nbyte;

	SceOff size_align = FAPS_DMASS_BLOCK_ALIGN(pEntry->stat.st_size);
	if(FAPS_DMASS_BLOCK_ALIGN(pEntry->stat.st_size + w_size) > size_align){

		ksceDebugPrintf("\tGrow data\n");

		void *temp_data = dmass_malloc(FAPS_DMASS_BLOCK_ALIGN(pEntry->stat.st_size + w_size));
		if(temp_data == NULL)
			return -2;

		memcpy(temp_data, pEntry->data, (size_t)pEntry->stat.st_size);

		dmass_free(pEntry->data);
		pEntry->data = temp_data;
	}

	memcpy(pEntry->data + argp->offset, argp->data, w_size);

	if((argp->offset + w_size) > pEntry->stat.st_size){
		pEntry->stat.st_size     = argp->offset + w_size;
		argp->node->core.st_size = argp->offset + w_size;
	}

	return w_size;
}

int vop_inactive(SceVopInactiveArgs *argp){
	ksceDebugPrintf("%s\n", __FUNCTION__);
	ksceDebugPrintf("\tnode=%p\n", argp->node);

	if(argp->node == NULL)
		return 0x80010016;

	FapsDmassFs *pEntry = ((FapsDmassFs *)argp->node->core.dev_info);
	if(pEntry != NULL){
		pEntry->pNode = NULL;
	}

	argp->node->core.dev_info = NULL;

	return 0;
}

int vop_func54(void *argp){
	ksceDebugPrintf("%s\n", __FUNCTION__);
	return 0x80010016;
}

int vop_func58(void *argp){
	ksceDebugPrintf("%s\n", __FUNCTION__);
	return 0x80010016;
}

int vop_sync(SceVopSyncArgs *argp){
	ksceDebugPrintf("%s\n", __FUNCTION__);

	if(argp->flags != 0)
		return 0x8002000A; // SCE_KERNEL_ERROR_INVALID_FLAGS

	return 0;
}

int vop_get_stat_by_fd(SceVfsStatByFd *argp){
	ksceDebugPrintf("%s\n", __FUNCTION__);

	if(argp->node->core.dev_info == NULL)
		return -2;

	memcpy(argp->stat, &(((FapsDmassFs *)argp->node->core.dev_info)->stat), sizeof(*(argp->stat)));

	return 0;
}

int vop_chstat_by_fd(SceVopChstatByFdArgs *argp){
	ksceDebugPrintf("%s\n", __FUNCTION__);

	FapsDmassFs *pEntry;

	pEntry = ((FapsDmassFs *)argp->pNode->core.dev_info);
	if(pEntry == NULL)
		return -1;

	if((argp->bit & SCE_CST_MODE) != 0){

		if((pEntry->stat.st_mode & SCE_S_IFMT) != (argp->stat->st_mode & SCE_S_IFMT))
			return -1;

		pEntry->stat.st_mode = argp->stat->st_mode;
	}

	if((argp->bit & SCE_CST_SIZE) != 0){
		if(argp->stat->st_size < 0LL)
			return -1;

		if(argp->stat->st_size == 0LL){
			dmass_free(pEntry->data);
			pEntry->data = NULL;
		}else{
			if(argp->stat->st_size > 0xFFFFFFFF)
				return -1;

			void *temp_data = dmass_malloc((SceSize)argp->stat->st_size);
			if(temp_data == NULL)
				return -2;

			memcpy(temp_data, pEntry->data, (size_t)pEntry->stat.st_size);

			dmass_free(pEntry->data);
			pEntry->data = temp_data;
		}

		pEntry->stat.st_size     = argp->stat->st_size;
		argp->pNode->core.st_size = argp->stat->st_size;
	}

	if((argp->bit & SCE_CST_CT) != 0)
		memcpy(&(pEntry->stat.st_ctime), &(argp->stat->st_ctime), sizeof(SceDateTime));

	if((argp->bit & SCE_CST_AT) != 0)
		memcpy(&(pEntry->stat.st_atime), &(argp->stat->st_atime), sizeof(SceDateTime));

	if((argp->bit & SCE_CST_MT) != 0)
		memcpy(&(pEntry->stat.st_mtime), &(argp->stat->st_mtime), sizeof(SceDateTime));

	return 0;
}

int vop_func68(void *argp){
	ksceDebugPrintf("%s\n", __FUNCTION__);
	return 0x80010016;
}

int vop_func6C(void *argp){
	ksceDebugPrintf("%s\n", __FUNCTION__);
	return 0x80010016;
}

const SceVfsTable vfs_table_test = {
	.func00       = vfs_mount_func,
	.func04       = vfs_umount_func,
	.vfs_set_root = vfs_set_root,
	.func0C       = NULL,
	.func10       = NULL,
	.func14       = NULL,
	.func18       = NULL,
	.func1C       = NULL,
	.func20       = vfs_init,
	.func24       = vfs_deinit,
	.func28       = NULL,
	.vfs_devctl   = vfs_devctl,
	.func30       = NULL // PathElme
};

const SceVopTable vop_table_test = {
	.vop_open           = vop_open,
	.vop_create         = vop_create,
	.vop_close          = vop_close,
	.vop_lookup         = vop_part_init,// ok?
	.vop_read           = vop_read,
	.vop_write          = vop_write,
	.vop_lseek          = vop_lseek,
	.vop_ioctl          = vop_ioctl,
	.vop_remove         = vop_remove,
	.vop_mkdir          = vop_mkdir,
	.vop_rmdir          = vop_rmdir,
	.vop_dopen          = vop_dopen,
	.vop_dclose         = vop_dclose,
	.vop_dread          = vop_dread,
	.vop_get_stat       = vop_get_stat,
	.vop_chstat         = vop_chstat,
	.vop_rename         = vop_rename,
	.func44             = NULL,
	.vop_pread          = vop_pread,
	.vop_pwrite         = vop_pwrite,
	.vop_inactive       = vop_inactive,// ok?
	.func54             = vop_func54,// TODO
	.func58             = vop_func58,// TODO
	.vop_sync           = vop_sync,
	.vop_get_stat_by_fd = vop_get_stat_by_fd,
	.vop_chstat_by_fd   = vop_chstat_by_fd,
	.func68             = vop_func68,// TODO
	.func6C             = vop_func6C,// TODO
	.func70             = NULL
};

const SceVfsMount2 vfs_mount2_devkit_test = {
	.unit      = FAPS_DMASS_DEVICE_NAME,
	.device1   = "faps_dmass_fs",
	.device2   = "faps_dmass_fs",
	.device3   = NULL,
	.data_0x10 = 0
};

const SceVfsMountParam vfs_mount_devkit_test = {
	.device    = "/md",
	.data_0x04 = 0,
	// .data_0x08 = 0x03000010, // cannot read? seems write only.
	.data_0x08 = 0x03000004,
	.data_0x0C = 0x00008003,
	.data_0x10 = "faps_dmass_fs",
	.data_0x14 = 0,
	.pVfsMount2 = &vfs_mount2_devkit_test,
	.data_0x1C = 0
};

#define TEST_MEMORY_SIZE (0x400000)

int dmass_create_process_resource(SceUID pid){

	FapsDmassProcessContext *pContext;

	int res = dmass_create_proc_context(pid, &pContext);
	if(res >= 0){
		ksceDebugPrintf("Proc context ok\n");
	}

	ksceDebugPrintf("pid=0x%08X\n", ksceKernelGetProcessId());
	ksceDebugPrintf("TLS pid=0x%08X\n", ksceKernelGetProcessIdFromTLS());

	pContext->memid = ksceKernelAllocMemBlock("dmass_test_memory", 0x1050D006, TEST_MEMORY_SIZE, NULL);
	// pContext->memid = ksceKernelAllocMemBlock("dmass_test_memory", 0x10C0D006, 0x100000, NULL);
	ksceDebugPrintf("sceKernelAllocMemBlock = 0x%X\n", pContext->memid);
	if(pContext->memid >= 0){
		ksceKernelGetMemBlockBase(pContext->memid, &pContext->membase);

		uintptr_t paddr;
		res = ksceKernelVAtoPA(pContext->membase, &paddr);
		if(res >= 0){
			ksceDebugPrintf("paddr=%p\n", paddr);
		}
	}

/*
	if(pid != SCE_GUID_KERNEL_PROCESS_ID){
		pContext->heap_id = -1;
	}else{
		pContext->heap_id = -1;
	}
*/

	return 0;
}

int dmass_delete_process_resource(SceUID pid){

	int res;
	FapsDmassProcessContext *pContext;

	res = dmass_search_proc_context(SCE_GUID_KERNEL_PROCESS_ID, &pContext);
	if(res >= 0 && pContext->membase != NULL){
		memset(pContext->membase, 0xAA, TEST_MEMORY_SIZE);
	}

	res = dmass_search_proc_context(pid, &pContext);
	if(res < 0){
		return res;
	}

	if(pContext->memid >= 0){
		memset(pContext->membase, 0xAA, TEST_MEMORY_SIZE);
		ksceKernelFreeMemBlock(pContext->memid);
		pContext->memid = -1;
	}

	res = dmass_delete_proc_context(pid);
	if(res < 0){
		return res;
	}

	pContext = NULL;

	ksceDebugPrintf("Proc context del ok\n");

	return 0;
}

int dmass_proc_create(SceUID pid, SceProcEventInvokeParam2 *a2, int a3){
	dmass_create_process_resource(pid);
	return 0;
}

int dmass_proc_exit(SceUID pid, SceProcEventInvokeParam1 *a2, int a3){
	dmass_delete_process_resource(pid);
	return 0;
}

int dmass_proc_kill(SceUID pid, SceProcEventInvokeParam1 *a2, int a3){
	dmass_delete_process_resource(pid);
	return 0;
}

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp){

	int res;

	res = dmass_heap_init();
	if(res < 0)
		return SCE_KERNEL_START_FAILED;

/*
	SceProcEventHandler proc_handler;
	proc_handler.size           = sizeof(proc_handler);
	proc_handler.create         = dmass_proc_create;
	proc_handler.exit           = dmass_proc_exit;
	proc_handler.kill           = dmass_proc_kill;
	proc_handler.stop           = NULL;
	proc_handler.start          = NULL;
	proc_handler.switch_process = NULL;

	ksceKernelRegisterProcEventHandler("FapsDmassProcEvent", &proc_handler, 0);
*/

	vfs_add.pVfsTable  = &vfs_table_test;
	vfs_add.device     = "faps_dmass_fs";
	vfs_add.data_0x08  = 0x11;
	vfs_add.is_mounted = 0;
	vfs_add.data_0x10  = 0x10;
	vfs_add.pVopTable  = &vop_table_test;
	vfs_add.data_0x18  = 0;

	res = ksceVfsAddVfs(&vfs_add);
	if(res < 0){
		ksceDebugPrintf("%s=0x%X\n", "sceVfsAddVfs", res);
		return SCE_KERNEL_START_SUCCESS;
	}

	res = ksceVfsMount(&vfs_mount_devkit_test);
	if(res < 0){
		ksceDebugPrintf("%s=0x%X\n", "sceVfsMount", res);
		return SCE_KERNEL_START_SUCCESS;
	}

	// dmass_create_process_resource(SCE_GUID_KERNEL_PROCESS_ID);

	SceUID fd;

	fd = ksceIoOpen("vsd0:file.bin", SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0666);
	ksceIoWrite(fd, &vfs_add, sizeof(vfs_add));
	ksceIoClose(fd);



	return SCE_KERNEL_START_SUCCESS;
}
