
#include <psp2kern/types.h>
#include <psp2kern/kernel/sysclib.h>
#include <psp2kern/kernel/sysmem.h>
#include <psp2kern/kernel/rtc.h>
#include <psp2kern/io/stat.h>
#include "fs_tool.h"

FapsDmassFs *pTree = NULL;

SceUID test_heap_uid = -1;

int dmass_heap_init(void){

	int res = ksceKernelCreateHeap("FapsVfsHeap", 0x10000, NULL);
	if(res < 0)
		return res;

	test_heap_uid = res;

	return 0;
}

void *dmass_malloc(SceSize length){
	return ksceKernelAllocHeapMemory(test_heap_uid, length);
}

int dmass_free(void *ptr){
	return ksceKernelFreeHeapMemory(test_heap_uid, ptr);
}

int set_fs_root(FapsDmassFs *pEntry){

	if(pTree != NULL)
		return -1;

	pTree = pEntry;

	return 0;
}

/*
int dmass_data_alloc(FapsDmassFs *pEntry, SceSize size){

	FapsDmassFileInfo *pFileInfo;


	pFileInfo = ksceKernelAllocHeapMemory(test_heap_uid, sizeof(*pFileInfo));
	if(pFileInfo == NULL)
		return -1;

	pFileInfo->pEntry = pEntry;

	while(size >= 0){
		SceSize req_size = (size >= 0x200000) ? 0x200000 : size;

		pFileInfo->nPages++;

		size -= req_size;
	}

	return 0;
}
*/
char *name_cpy(const char *s){

	int len = strlen(s);
	if(len == 0)
		return NULL;

	char *res = ksceKernelAllocHeapMemory(test_heap_uid, len + 1);
	if(res == NULL)
		return NULL;

	res[len] = 0;
	memcpy(res, s, len);
	return res;
}

int get_fs_entry_internal(const char *path, FapsDmassFs **ppEntry){

	if(path == NULL || path[0] != '/')
		return -1;

	const char *current = path;

	int level = 0;

	FapsDmassFs *pTreeLoc = pTree;

	while(current[0] != 0){
		char *s1 = strchr(current, '/');
		if(s1 == NULL)
			return -2;

		char *s2 = strchr(s1 + 1, '/');
		if(s2 == NULL)
			s2 = strchr(s1 + 1, 0);

		int len = s2 - s1 - 1;
		if(len == 0) // "//file.bin"
			return -2;

		char name[0x40];

		if(len <= 0x3F && 0){
			name[len] = 0;
			memcpy(name, s1 + 1, len);
			ksceDebugPrintf("%s:level=%02d:name=%s\n", __FUNCTION__, level, name);
		}

		while(pTreeLoc != NULL){
			if(memcmp(pTreeLoc->name, s1 + 1, len) == 0 && pTreeLoc->name[len] == 0){
				goto next;
			}

			pTreeLoc = pTreeLoc->next;
		}

		return 0x80010002;

next:
		if(s2[0] == '/') // have next path
			pTreeLoc = pTreeLoc->next_dir;

		level++;
		current = s2;
	}

	if(ppEntry != NULL)
		*ppEntry = pTreeLoc;

	return 0;
}

int get_fs_entry(const char *path, FapsDmassFs **ppEntry){

	int res;

	res = get_fs_entry_internal(path, ppEntry);

	// ksceDebugPrintf("\t%s=0x%X\n", __FUNCTION__, res);

	return res;
}

int check_name_internal(char ch, const char *list){

	const char *current = list;

	while(*current != 0){
		if(ch == *current)
			return 0x80020005;

		current++;
	}

	return 0;
}

int check_name(const char *name){

	int res;
	const char *current = name;

	while(*current != 0){

		res = check_name_internal(*current, "\\/:*?\"<>|");
		if(res < 0)
			return res;

		current++;
	}

	return 0;	
}

int init_entry_date(SceIoStat *stat){

	int res;
	SceRtcTick tick;
	SceDateTime datetime;
	res = ksceRtcGetCurrentTick(&tick);
	if(res < 0)
		return res;

	res = ksceRtcConvertTickToDateTime(&datetime, &tick);
	if(res < 0)
		return res;

	memset(stat, 0, sizeof(*stat));
	stat->st_mode = 0;
	stat->st_attr = 0;
	stat->st_size = 0LL;
	memcpy(&stat->st_ctime, &datetime, sizeof(stat->st_ctime));
	memcpy(&stat->st_atime, &datetime, sizeof(stat->st_atime));
	memcpy(&stat->st_mtime, &datetime, sizeof(stat->st_mtime));

	return 0;
}

FapsDmassFs *create_dmass_file_info(const char *name){

	char *new_name;
	FapsDmassFs *pDmassFs;

	pDmassFs = ksceKernelAllocHeapMemory(test_heap_uid, sizeof(*pDmassFs));
	if(pDmassFs == NULL)
		return NULL;

	new_name = name_cpy(name);
	if(new_name == NULL){
		ksceKernelFreeHeapMemory(test_heap_uid, pDmassFs);
		return NULL;
	}

	memset(pDmassFs, 0, sizeof(*pDmassFs));

	pDmassFs->next         = NULL;
	pDmassFs->next_dir     = NULL;
	pDmassFs->pNode        = NULL;
	pDmassFs->pDmassFsX    = NULL;
	pDmassFs->refer        = 0;
	pDmassFs->name         = name_cpy(name);
	init_entry_date(&pDmassFs->stat);
	pDmassFs->stat.st_mode = SCE_S_IWUSR | SCE_S_IRUSR | SCE_S_IFREG;
	pDmassFs->data         = NULL;

	ksceDebugPrintf("File created by 0x%08X\n", ksceKernelGetProcessId());

	return pDmassFs;
}

FapsDmassFs *create_dmass_dir_info(const char *name){

	char *new_name;
	FapsDmassFs *pDmassFs;

	pDmassFs = ksceKernelAllocHeapMemory(test_heap_uid, sizeof(*pDmassFs));
	if(pDmassFs == NULL)
		return NULL;

	new_name = name_cpy(name);
	if(new_name == NULL){
		ksceKernelFreeHeapMemory(test_heap_uid, pDmassFs);
		return NULL;
	}

	memset(pDmassFs, 0, sizeof(*pDmassFs));

	pDmassFs->next         = NULL;
	pDmassFs->next_dir     = NULL;
	pDmassFs->pNode        = NULL;
	pDmassFs->pDmassFsX    = NULL;
	pDmassFs->refer        = 0;
	pDmassFs->name         = name_cpy(name);
	init_entry_date(&pDmassFs->stat);
	pDmassFs->stat.st_mode = SCE_S_IWUSR | SCE_S_IRUSR | SCE_S_IFDIR;
	pDmassFs->data         = NULL;

	return pDmassFs;
}

int delete_dmass_info(FapsDmassFs *pDmassFs){

	if(pDmassFs->refer != 0){
		ksceDebugPrintf("%s: Has other refer\n", __FUNCTION__);
		return -1;
	}

	if(pDmassFs->pDmassFsX != NULL){
		ksceDebugPrintf("%s: Has other refer object\n", __FUNCTION__);
		return -2;
	}

	ksceKernelFreeHeapMemory(test_heap_uid, pDmassFs->name);
	pDmassFs->name = NULL;

	ksceKernelFreeHeapMemory(test_heap_uid, pDmassFs->data);
	pDmassFs->data = NULL;

	ksceKernelFreeHeapMemory(test_heap_uid, pDmassFs);

	return 0;
}

FapsDmassFsX *create_dmass_ctl_data(FapsDmassFs *pDmassFs, int key){

	FapsDmassFsX *pObject;

	pObject = ksceKernelAllocHeapMemory(test_heap_uid, sizeof(*pObject));
	if(pObject == NULL)
		return NULL;

	pObject->next          = pDmassFs->pDmassFsX;
	pObject->handler_index = key;
	pObject->current       = NULL;

	pDmassFs->pDmassFsX = pObject;
	pDmassFs->refer++;

	return pObject;
}

int delete_dmass_ctl_data(FapsDmassFs *pDmassFs, int key){

	FapsDmassFsX **ppObject = &(pDmassFs->pDmassFsX);

	while(*ppObject != NULL){
		if(key == (*ppObject)->handler_index){

			FapsDmassFsX *pObject = *ppObject;

			*ppObject = (*ppObject)->next;
			pDmassFs->refer--;

			ksceKernelFreeHeapMemory(test_heap_uid, pObject);

			return 0;
		}

		ppObject = &((*ppObject)->next);
	}

	return -2;
}

FapsDmassFsX *search_dmass_ctl_data(FapsDmassFs *pDmassFs, int key){

	FapsDmassFsX *pObject = pDmassFs->pDmassFsX;

	while(pObject != NULL){
		if(key == pObject->handler_index){
			return pObject;
		}

		pObject = pObject->next;
	}

	return NULL;
}

/*
int print_fs_tree_internal(int level, const FapsDmassFs *pDmassFs){

	const FapsDmassFs *tree = pDmassFs;
	while(tree != NULL){

		if(tree->next_dir != NULL){
			ksceDebugPrintf("%02d:%s:%s\n", level, "d", tree->name);
			return print_fs_tree_internal(level + 1, tree->next_dir);
		}

		ksceDebugPrintf("%02d:%s:%s\n", level, "f", tree->name);
		tree = tree->next;
	}

	return 0;
}

int print_fs_tree(void){

	print_fs_tree_internal(0, pTree);

	return 0;
}

int free_fs_vfs_internal(FapsDmassFs *pDmassFs){

	const FapsDmassFs *tree = pDmassFs;
	while(tree != NULL){

		SceVfsNode *pNode = tree->pNode;

		if(pNode != NULL && pNode->core.some_counter_58 == 0){
			ksceVopInactive(pNode);
			ksceVfsFreeVnode(pNode);
		}

		if(tree->next_dir != NULL){
			return free_fs_vfs_internal(tree->next_dir);
		}

		tree = tree->next;
	}

	return 0;
}

int free_fs_vfs(void){

	free_fs_vfs_internal(pTree->next_dir);

	return 0;
}
*/
