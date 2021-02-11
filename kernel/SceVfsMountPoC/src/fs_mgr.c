/*
 * SceVfs mount PoC
 * Copyright (C) 2020 Princess of Sleeping
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include <psp2kern/kernel/sysmem.h>
#include <psp2kern/io/fcntl.h>
#include <string.h>
#include "fs_mgr.h"
#include "item_mgr.h"
#include "vfs.h"

extern void *root_node;
extern SceUID heap_uid;

int PathCheck(const char *path, int path_len, char c){

	for(int i=0;i<path_len;i++){
		if(path[i] == c)
			return -1;
	}

	return 0;
}

int createFileEntry(const char *path, FileEntry **ppDirEntry){

	DataPack_t *pDataPack = getFileEntryByNode(root_node);
	if(pDataPack == NULL){
		ksceDebugPrintf("getFileEntryByNode failed\n");
		return -1;
	}

	char *name;
	FileEntry *dir_entry = pDataPack->dir_entry_root;
	FileEntry **ppEntry, *pNext;

	ppEntry = &pDataPack->dir_entry_root;

	while(path[0] != '0'){

		SceSize path_len = strlen(path);

		if(dir_entry == NULL){
			ksceDebugPrintf("dir_entry == NULL : failed\n");
			ksceDebugPrintf("%s\n", path);

			if(PathCheck(path, path_len, '/') < 0)
				return -1;

			FileEntry *entry = ksceKernelAllocHeapMemory(heap_uid, sizeof(FileEntry));
			if(entry == NULL)
				return -1;

			memset(entry, 0, sizeof(FileEntry));

			*ppEntry = entry;

			name = ksceKernelAllocHeapMemory(heap_uid, path_len + 1);
			name[path_len] = 0;
			memcpy(name, path, path_len);

			entry->name = name;

			*ppDirEntry = entry;

			return 0;
		}

		ppEntry = &dir_entry->next;

		if(strncmp(path, dir_entry->name, strlen(dir_entry->name)) == 0){

			if(path[strlen(dir_entry->name)] == '/' && (dir_entry->stat->st_mode & SCE_S_IFDIR) != 0){

				path = &path[strlen(dir_entry->name) + 1];

				ppEntry = &dir_entry->next_dir;

				dir_entry = dir_entry->next_dir;
				continue;
			}

			if(path[strlen(dir_entry->name)] != '/'){
				dir_entry = dir_entry->next;
				continue;
			}

			if(path[strlen(dir_entry->name)] == 0){
				return -1;
			}

			pNext = dir_entry->next;

			FileEntry *entry = ksceKernelAllocHeapMemory(heap_uid, sizeof(FileEntry));
			memset(entry, 0, sizeof(FileEntry));

			*ppEntry = entry;

			entry->next = pNext;

			name = ksceKernelAllocHeapMemory(heap_uid, path_len + 1);
			name[path_len] = 0;
			memcpy(name, path, path_len);

			(*ppEntry)->name = name;

			*ppDirEntry = *ppEntry;

			return 0;
		}

		dir_entry = dir_entry->next;
	}

	return -1;
}

int getDirEntry(const char *path, FileEntry **ppDirEntry){

	const DataPack_t *pDataPack = getFileEntryByNode(root_node);
	if(pDataPack == NULL){
		ksceDebugPrintf("getFileEntryByNode failed\n");
		ksceDebugPrintf("%s\n", path);
		return -1;
	}

	FileEntry *dir_entry = pDataPack->dir_entry_root, *e2;

	e2 = dir_entry;

	while(path[0] != '0'){

		e2 = dir_entry;

		if(dir_entry == NULL){
			ksceDebugPrintf("dir_entry == NULL : failed\n");
			ksceDebugPrintf("%s\n", path);
			return -1;
		}

		if(strncmp(path, dir_entry->name, strlen(dir_entry->name)) == 0){

			if(path[strlen(dir_entry->name)] == 0 && (dir_entry->stat->st_mode & SCE_S_IFDIR) != 0){

				if(ppDirEntry != NULL)
					*ppDirEntry = e2;

				return 0;
			}

			if(path[strlen(dir_entry->name)] == '/'){

				path = &path[strlen(dir_entry->name) + 1];

				dir_entry = dir_entry->next_dir;
				continue;
			}

			// ksceDebugPrintf("getDirEntry not found : %s\n", path);
			return -1;
		}

		dir_entry = dir_entry->next;
	}

	if(ppDirEntry != NULL)
		*ppDirEntry = e2;

	return 0;
}

int getFileEntry(const char *path, FileEntry **ppDirEntry){

	DataPack_t *pDataPack = getFileEntryByNode(root_node);
	if(pDataPack == NULL){
		ksceDebugPrintf("getFileEntryByNode failed\n");
		return -1;
	}

	FileEntry *dir_entry = pDataPack->dir_entry_root, *e2;

	e2 = dir_entry;

	while(path[0] != '0'){

		e2 = dir_entry;

		if(dir_entry == NULL){
			ksceDebugPrintf("dir_entry == NULL : failed\n");
			return -1;
		}

		if(strncmp(path, dir_entry->name, strlen(dir_entry->name)) == 0){

			if(path[strlen(dir_entry->name)] == 0 && (dir_entry->stat->st_mode & SCE_S_IFREG) != 0){

				if(ppDirEntry != NULL)
					*ppDirEntry = e2;

				return 0;
			}

			if(path[strlen(dir_entry->name)] == '/' && (dir_entry->stat->st_mode & SCE_S_IFDIR) != 0){

				path = &path[strlen(dir_entry->name) + 1];

				dir_entry = dir_entry->next_dir;
				continue;
			}

			return -1;
		}

		dir_entry = dir_entry->next;
	}

	if(ppDirEntry != NULL)
		*ppDirEntry = e2;

	return 0;
}

const SceIoStat stat_file_tpl = {
	.st_mode = SCE_S_IFREG | SCE_S_IROTH | SCE_S_IRUSR | SCE_S_IWUSR,
	.st_attr = SCE_SO_IFREG,
	.st_size = 0LL,

	.st_ctime.year        = 2020,
	.st_ctime.month       = 7,
	.st_ctime.day         = 5,
	.st_ctime.hour        = 1,
	.st_ctime.minute      = 33,
	.st_ctime.second      = 0,
	.st_ctime.microsecond = 0,

	.st_atime.year        = 2020,
	.st_atime.month       = 7,
	.st_atime.day         = 5,
	.st_atime.hour        = 1,
	.st_atime.minute      = 33,
	.st_atime.second      = 0,
	.st_atime.microsecond = 0,

	.st_mtime.year        = 2020,
	.st_mtime.month       = 7,
	.st_mtime.day         = 5,
	.st_mtime.hour        = 1,
	.st_mtime.minute      = 33,
	.st_mtime.second      = 0,
	.st_mtime.microsecond = 0
};

const SceIoStat stat_dir_tpl = {
	.st_mode = SCE_S_IFDIR | SCE_S_IRUSR | SCE_S_IWUSR,
	.st_attr = 0,
	.st_size = 0LL,

	.st_ctime.year        = 2020,
	.st_ctime.month       = 7,
	.st_ctime.day         = 5,
	.st_ctime.hour        = 1,
	.st_ctime.minute      = 33,
	.st_ctime.second      = 0,
	.st_ctime.microsecond = 0,

	.st_atime.year        = 2020,
	.st_atime.month       = 7,
	.st_atime.day         = 5,
	.st_atime.hour        = 1,
	.st_atime.minute      = 33,
	.st_atime.second      = 0,
	.st_atime.microsecond = 0,

	.st_mtime.year        = 2020,
	.st_mtime.month       = 7,
	.st_mtime.day         = 5,
	.st_mtime.hour        = 1,
	.st_mtime.minute      = 33,
	.st_mtime.second      = 0,
	.st_mtime.microsecond = 0
};

int setToDirEntry(FileEntry *entry){

	entry->open_allow_flags = SCE_O_RDONLY | SCE_O_DIROPEN;
	entry->stat = ksceKernelAllocHeapMemory(heap_uid, sizeof(SceIoStat));

	memcpy(entry->stat, &stat_dir_tpl, sizeof(SceIoStat));

	return 0;
}

int setToFileEntryTplStat(FileEntry *entry){

	memcpy(entry->stat, &stat_file_tpl, sizeof(SceIoStat));

	return 0;
}
