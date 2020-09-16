/*
 * SceVfs mount PoC
 * Copyright (C) 2020 Princess of Sleeping
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/sysmem.h>
#include <psp2kern/io/fcntl.h>
#include <stdio.h>
#include <string.h>
#include "vfs.h"
#include "sce_cpu.h"
#include "fs_mgr.h"
#include "item_mgr.h"
#include "etc.h"

extern SceUID heap_uid;

int ksceKernelGetRandomNumber(void *dst, SceSize size);

int read_random_cb(DataPack_t *data, const void *args, void *pData, SceSize size){

	int readable_byte = data->size - data->seek;
	if(readable_byte == 0)
		return readable_byte;

	if(size > readable_byte)
		size = readable_byte;

#define ksceSblRngPseudoRandomNumber ksceKernelGetRandomNumber

	ksceSblRngPseudoRandomNumber(pData, size);

	data->seek += size;

	return (int)size;
}

int init_etc(void){

	FileEntry *entry;

	createFileEntry("etc", &entry);
	setToDirEntry(entry);

	createFileEntry("etc/random", &entry);

	entry->open_allow_flags = SCE_O_RDONLY;
	entry->size     = (SceOff)0x1000LL;

	entry->stat = ksceKernelAllocHeapMemory(heap_uid, sizeof(SceIoStat));

	setToFileEntryTplStat(entry);
	entry->stat->st_size = (SceOff)0x1000LL;

	entry->read_cb        = (ReadCb)read_random_cb;

	return 0;
}
