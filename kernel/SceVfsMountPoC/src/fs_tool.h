

#ifndef _FAPS_VFS_FS_TOOL_H_
#define _FAPS_VFS_FS_TOOL_H_

#include <psp2kern/types.h>
#include "vfs.h"

typedef struct FapsDmassFs FapsDmassFs;

typedef struct FapsDmassFsX {
	struct FapsDmassFsX *next;
	int handler_index;
	FapsDmassFs *current; // for dread
} FapsDmassFsX;

typedef struct FapsDmassFs {
	struct FapsDmassFs *next;
	struct FapsDmassFs *next_dir;
	SceVfsNode *pNode;
	FapsDmassFsX *pDmassFsX;
	SceUInt32 refer;
	char *name;
	SceIoStat stat;
	void *data;
} FapsDmassFs;

int dmass_heap_init(void);
void *dmass_malloc(SceSize length);
int dmass_free(void *ptr);

int set_fs_root(FapsDmassFs *pEntry);

FapsDmassFs *create_dmass_file_info(const char *name);
FapsDmassFs *create_dmass_dir_info(const char *name);
int delete_dmass_info(FapsDmassFs *pDmassFs);

FapsDmassFsX *create_dmass_ctl_data(FapsDmassFs *pDmassFs, int key);
int delete_dmass_ctl_data(FapsDmassFs *pDmassFs, int key);
FapsDmassFsX *search_dmass_ctl_data(FapsDmassFs *pDmassFs, int key);

char *name_cpy(const char *s);
int get_fs_entry(const char *path, FapsDmassFs **ppEntry);
int check_name(const char *name);
int init_entry_date(SceIoStat *stat);

#endif // _FAPS_VFS_FS_TOOL_H_
