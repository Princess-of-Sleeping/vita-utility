

#ifndef _FAPS_DMASS_PROCESS_H_
#define _FAPS_DMASS_PROCESS_H_

#include <psp2kern/types.h>

/*
typedef struct FapsDmassFileData {
	struct FapsDmassFileData *next;
	SceUID memid;
	void *data;
	SceSize size;
} FapsDmassFileData;

typedef struct FapsDmassFileInfo {
	struct FapsDmassFileInfo *next;
	FapsDmassFs *pEntry;
	FapsDmassFileData *data;
	SceUInt32 nPages;
} FapsDmassFileInfo;
*/

typedef struct FapsDmassProcessContext {
	struct FapsDmassProcessContext *next;
	SceUID pid;
	SceUID memid;
	void *membase;
	SceUID heap_id;
	// FapsDmassFileInfo *pFileInfo:
} FapsDmassProcessContext;

int dmass_create_proc_context(SceUID pid, FapsDmassProcessContext **ppContext);
int dmass_delete_proc_context(SceUID pid);
int dmass_search_proc_context(SceUID pid, FapsDmassProcessContext **ppContext);

#endif // _FAPS_DMASS_PROCESS_H_
