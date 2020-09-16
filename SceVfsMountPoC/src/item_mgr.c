/*
 * SceVfs mount PoC
 * Copyright (C) 2020 Princess of Sleeping
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include <psp2kern/kernel/sysmem.h>
#include <string.h>
#include "fs_mgr.h"
#include "item_mgr.h"
#include "vfs.h"

extern SceUID (* sceKernelCreateUidObj)(SceClass *cls, const char *name, SceCreateUidObjOpt *opt, SceObjectBase **obj);

DataPack_t *last_obj = NULL;
SceClass g_class;

int init_patch(DataPackForSystem_t *pNewObject){

	memset(&pNewObject->data, 0, sizeof(DataPack_t));

	if(last_obj != NULL){
		last_obj->prev = &pNewObject->data;
	}

	pNewObject->data.next = last_obj;

	last_obj = &pNewObject->data;

	return 0;
}

int free_patch(DataPackForSystem_t *pDelObject){

	if(pDelObject->data.prev == NULL){
		last_obj = pDelObject->data.next;
	}else{
		pDelObject->data.prev->next = pDelObject->data.next;
	}

	if(pDelObject->data.next != NULL){
		pDelObject->data.next->prev = pDelObject->data.prev;
	}

	return 0;
}

void *getFileEntryByNode(const SceVfsNode *node){

	DataPack_t *LastObj = last_obj;

	while(LastObj != NULL){
		if(LastObj->node == node){
			return LastObj;
		}

		LastObj = LastObj->next;
	}

	return NULL;
}

int create_file_item(void *node, DataPackForSystem_t **ppDataPackForSystem){

	SceUID res;
	DataPackForSystem_t *pDataPackForSystem;

	res = sceKernelCreateUidObj(&g_class, "FileItem", NULL, (SceObjectBase **)&pDataPackForSystem);
	if(res < 0)
		return res;

	pDataPackForSystem->data.uid = res;
	pDataPackForSystem->data.node = node;

	*ppDataPackForSystem = pDataPackForSystem;

	// ksceDebugPrintf("create node item 0x%08X node:0x%08X 0x%X\n", pDataPackForSystem, node, res);

	return 0;
}

int init_itemmgr(void){

	ksceKernelCreateClass(&g_class, "SceTestClass", ksceKernelGetUidClass(), sizeof(DataPackForSystem_t), (SceClassCallback)init_patch, (SceClassCallback)free_patch);

	return 0;
}
