/*
 * SceVfs mount PoC
 * Copyright (C) 2020 Princess of Sleeping
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef _PSP2_FS_MGR_H_
#define _PSP2_FS_MGR_H_

#include <psp2kern/types.h>
#include <psp2kern/io/stat.h>

typedef struct DataPack_t DataPack_t;
typedef struct FileEntry FileEntry;

typedef int (* OpenCb)(DataPack_t *data, const void *args);
typedef int (* ReadCb)(DataPack_t *data, const void *args, void *pData, SceSize size);
typedef int (* WriteCb)(DataPack_t *data, const void *args, const void *pData, SceSize size);
typedef int (* GetStatCb)(const FileEntry *entry, const void *args, SceIoStat *stat);

typedef int (* DopenCb)(DataPack_t *data, const void *args);

typedef struct FileEntry {
	struct FileEntry *next;
	struct FileEntry *next_dir;
	int flags;
	int open_allow_flags;

	const char *name;
	SceIoStat *stat;
	SceOff size;

	OpenCb open_cb;
	ReadCb read_cb;
	WriteCb write_cb;
	GetStatCb get_io_stat_cb;
	DopenCb dopen_cb;

	const void *args_for_stat;
	int pad[3];
} FileEntry;

typedef struct DataPack_t {
	struct DataPack_t *next;
	struct DataPack_t *prev;
	SceUID uid;
	void *node;

	SceOff seek;
	SceOff size;

	FileEntry *dir_entry_root;
	FileEntry *dir_entry;
} DataPack_t;

typedef struct DataPackForSystem_t {
	int sce_reserved[2];
	DataPack_t data;
} DataPackForSystem_t;

int createFileEntry(const char *path, FileEntry **ppDirEntry);

int getDirEntry(const char *path, FileEntry **ppDirEntry);
int getFileEntry(const char *path, FileEntry **ppDirEntry);

int setToDirEntry(FileEntry *entry);
int setToFileEntryTplStat(FileEntry *entry);

#endif // _PSP2_FS_MGR_H_
