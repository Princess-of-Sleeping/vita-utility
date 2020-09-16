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
#include <psp2kern/io/fcntl.h>
#include <stdio.h>
#include <string.h>
#include "vfs.h"
#include "sce_cpu.h"
#include "fs_mgr.h"
#include "item_mgr.h"
#include "sysroot_kblparam.h"

extern SceUID heap_uid;

int read_kblparam_cb(DataPack_t *data, const void *args, void *pData, SceSize size){

	int readable_byte = data->size - data->seek;
	if(readable_byte == 0)
		return readable_byte;

	if(size > readable_byte)
		size = readable_byte;

	memcpy(pData, ksceKernelGetSysrootBuffer(), size);

	data->seek += size;

	return (int)size;
}

int write_kblparam_cb(DataPack_t *data, const void *args, const void *pData, SceSize size){

	int readable_byte = data->size - data->seek;
	if(readable_byte == 0)
		return readable_byte;

	if(size > readable_byte)
		size = readable_byte;

	int addr = 0, prev;

	prev = ksceKernelCpuSuspendIntr(&addr);

	memcpy(ksceKernelGetSysrootBuffer(), pData, size);

	ksceKernelCpuResumeIntr(&addr, prev);

	data->seek += size;

	return (int)size;
}

int init_sysroot(void){

	FileEntry *entry;

	createFileEntry("sysroot", &entry);
	setToDirEntry(entry);

	createFileEntry("sysroot/sysroot(kblparam)", &entry);

	entry->open_allow_flags = SCE_O_RDONLY | SCE_O_WRONLY;
	entry->size     = (SceOff)0x200LL;

	entry->stat = ksceKernelAllocHeapMemory(heap_uid, sizeof(SceIoStat));

	setToFileEntryTplStat(entry);
	entry->stat->st_size = (SceOff)0x200LL;

	entry->read_cb  = (ReadCb)read_kblparam_cb;
	entry->write_cb = (WriteCb)write_kblparam_cb;

	return 0;
}
