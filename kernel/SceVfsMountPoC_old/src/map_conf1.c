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
#include "map_conf1.h"

extern SceUID heap_uid;

uint32_t map_conf1_data[2];

int open_map_mem1_cb(DataPack_t *data, const void *args){

	data->size = (SceOff)map_conf1_data[1];

	return 0;
}

int get_io_stat_map_mem1_cb(const FileEntry *entry, const void *args, SceIoStat *stat){

	memcpy(stat, entry->stat, sizeof(SceIoStat));

	stat->st_size = (SceOff)map_conf1_data[1];

	return 0;
}


int read_map_mem1_cb(DataPack_t *data, const void *args, void *pData, SceSize size){

	int readable_byte = data->size - data->seek;
	if(readable_byte == 0)
		return readable_byte;

	if(size > readable_byte)
		size = readable_byte;

	memcpy(pData, (void *)map_conf1_data[0], size);

	data->seek += size;

	return (int)size;
}

int write_map_mem1_cb(DataPack_t *data, const void *args, const void *pData, SceSize size){

	int readable_byte = data->size - data->seek;
	if(readable_byte == 0)
		return readable_byte;

	if(size > readable_byte)
		size = readable_byte;

	int addr = 0, prev;

	prev = ksceKernelCpuSuspendIntr(&addr);

	memcpy((void *)map_conf1_data[0], pData, size);

	ksceKernelCpuResumeIntr(&addr, prev);

	data->seek += size;

	return (int)size;
}



int read_map_conf1_cb(DataPack_t *data, const void *args, void *pData, SceSize size){

	int readable_byte = data->size - data->seek;
	if(readable_byte == 0)
		return readable_byte;

	if(size > readable_byte)
		size = readable_byte;

	memcpy(pData, map_conf1_data, size);

	data->seek += size;

	return (int)size;
}

int write_map_conf1_cb(DataPack_t *data, const void *args, const void *pData, SceSize size){

	int readable_byte = data->size - data->seek;
	if(readable_byte == 0)
		return readable_byte;

	if(size > readable_byte)
		size = readable_byte;

	int addr = 0, prev;

	prev = ksceKernelCpuSuspendIntr(&addr);

	memcpy((void *)map_conf1_data, pData, size);

	ksceKernelCpuResumeIntr(&addr, prev);

	data->seek += size;

	return (int)size;
}

int init_map_conf1(void){

	FileEntry *entry;

	createFileEntry("map_conf", &entry);
	setToDirEntry(entry);

	createFileEntry("map_conf/map_conf1", &entry);

	entry->open_allow_flags = SCE_O_RDONLY | SCE_O_WRONLY;
	entry->size     = (SceOff)0x8LL;

	entry->stat = ksceKernelAllocHeapMemory(heap_uid, sizeof(SceIoStat));

	setToFileEntryTplStat(entry);
	entry->stat->st_size = (SceOff)0x8LL;

	entry->read_cb  = (ReadCb)read_map_conf1_cb;
	entry->write_cb = (WriteCb)write_map_conf1_cb;



	createFileEntry("map_conf/map_mem1", &entry);

	entry->open_allow_flags = SCE_O_RDONLY | SCE_O_WRONLY;
	entry->size     = (SceOff)0LL;

	entry->stat = ksceKernelAllocHeapMemory(heap_uid, sizeof(SceIoStat));

	setToFileEntryTplStat(entry);
	entry->stat->st_size = (SceOff)0LL;

	entry->open_cb  = (OpenCb)open_map_mem1_cb;
	entry->read_cb  = (ReadCb)read_map_mem1_cb;
	entry->write_cb = (WriteCb)write_map_mem1_cb;
	entry->get_io_stat_cb = (GetStatCb)get_io_stat_map_mem1_cb;

	return 0;
}
