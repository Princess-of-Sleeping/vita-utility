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

typedef struct ModuleStat {
	const char *name;
	int seg;
} ModuleStat;

extern SceUID heap_uid;

extern int (* sceKernelGetModuleList)(SceUID pid, int flags1, int flags2, SceUID *modids, size_t *num);
extern int (* sceKernelGetModuleInfo)(SceUID pid, SceUID modid, SceKernelModuleInfo *info);

int get_io_stat_module_cb(const FileEntry *entry, const ModuleStat *args, SceIoStat *stat){

	SceUID modid = ksceKernelSearchModuleByName(args->name);
	if(modid < 0)
		return modid;

	SceKernelModuleInfo info;
	memset(&info, 0, sizeof(info));
	info.size = sizeof(info);
	sceKernelGetModuleInfo(0x10005, modid, &info);

	memcpy(stat, entry->stat, sizeof(SceIoStat));

	stat->st_size = (SceOff)info.segments[args->seg].memsz;

	return 0;
}

int open_module_cb(DataPack_t *data, const ModuleStat *args){

	SceUID modid = ksceKernelSearchModuleByName(args->name);
	if(modid < 0)
		return modid;

	SceKernelModuleInfo info;
	memset(&info, 0, sizeof(info));
	info.size = sizeof(info);
	sceKernelGetModuleInfo(0x10005, modid, &info);

	data->size = (SceOff)info.segments[args->seg].memsz;

	return 0;
}

int read_module_cb(DataPack_t *data, const ModuleStat *args, void *pData, SceSize size){

	int readable_byte = data->size - data->seek;
	if(readable_byte == 0)
		return readable_byte;

	SceUID modid = ksceKernelSearchModuleByName(args->name);
	if(modid < 0)
		return modid;

	SceKernelModuleInfo info;
	memset(&info, 0, sizeof(info));
	info.size = sizeof(info);
	sceKernelGetModuleInfo(0x10005, modid, &info);

	if(size > readable_byte)
		size = readable_byte;

	int addr = 0, prev;
	prev = ksceKernelCpuSuspendIntr(&addr);

	memcpy(pData, info.segments[args->seg].vaddr + ((int)data->seek), size);

	ksceKernelCpuResumeIntr(&addr, prev);

	data->seek += size;

	return (int)size;
}

int write_module_cb(DataPack_t *data, const ModuleStat *args, const void *pData, SceSize size){

	int readable_byte = data->size - data->seek;
	if(readable_byte == 0)
		return readable_byte;

	SceUID modid = ksceKernelSearchModuleByName(args->name);
	if(modid < 0)
		return modid;

	SceKernelModuleInfo info;
	memset(&info, 0, sizeof(info));
	info.size = sizeof(info);
	sceKernelGetModuleInfo(0x10005, modid, &info);

	if(size > readable_byte)
		size = readable_byte;

	int addr = 0, prev;
	prev = ksceKernelCpuSuspendIntr(&addr);

	if(args->seg == 0){
		memcpy_rx(0xDEADBEEF, info.segments[args->seg].vaddr + ((int)data->seek), pData, size);
	}else{
		memcpy(info.segments[args->seg].vaddr, pData, size);
	}

	ksceKernelCpuResumeIntr(&addr, prev);

	data->seek += size;

	return (int)size;
}

int dopen_cb_module(DataPack_t *data, const void *args){

	FileEntry *entry_ptr = data->dir_entry_root;
	FileEntry **ppEntry = &data->dir_entry_root;

	while(entry_ptr != NULL){

		ksceKernelFreeHeapMemory(heap_uid, (void *)entry_ptr->name);
		ksceKernelFreeHeapMemory(heap_uid, (void *)entry_ptr->stat);

		ksceKernelFreeHeapMemory(heap_uid, (void *)((ModuleStat *)entry_ptr->args_for_stat)->name);

		if(entry_ptr->args_for_stat != NULL)
			ksceKernelFreeHeapMemory(heap_uid, (void *)entry_ptr->args_for_stat);

		FileEntry *entry_next_ptr = entry_ptr->next;

		ksceKernelFreeHeapMemory(heap_uid, (void *)entry_ptr);

		entry_ptr = entry_next_ptr;
	}

	(*ppEntry) = NULL;

	SceUID modids[0x80];
	SceSize num = 0x80;
	ModuleStat *module_args;
	FileEntry *module_info;
	char *module_name;

	sceKernelGetModuleList(0x10005, 0xFF, 1, modids, &num);

	for(int i=0;i<num;i++){

		SceKernelModuleInfo info;
		memset(&info, 0, sizeof(info));
		info.size = sizeof(info);
		sceKernelGetModuleInfo(0x10005, modids[i], &info);

		if(info.segments[0].memsz != 0){

			module_name = ksceKernelAllocHeapMemory(heap_uid, 0x20);
			module_args = ksceKernelAllocHeapMemory(heap_uid, sizeof(ModuleStat));
			module_info = ksceKernelAllocHeapMemory(heap_uid, sizeof(FileEntry));

			(*ppEntry) = module_info;
			ppEntry = &module_info->next;
			memset(module_info, 0, sizeof(FileEntry));

			memcpy(module_name, info.module_name, 0x1F);

			module_args->name = module_name;
			module_args->seg = 0;

			module_info->next     = NULL;
			module_info->next_dir = NULL;
			module_info->open_allow_flags = SCE_O_RDONLY | SCE_O_WRONLY;
			module_info->flags    = 0;

			module_name = ksceKernelAllocHeapMemory(heap_uid, 0x28);
			snprintf(module_name, 0x27, "%s text", module_args->name);
			module_info->name     = module_name;
			module_info->size     = (SceOff)info.segments[0].memsz;

			module_info->stat = ksceKernelAllocHeapMemory(heap_uid, sizeof(SceIoStat));

			setToFileEntryTplStat(module_info);
			module_info->stat->st_size = (SceOff)info.segments[0].memsz;

			module_info->open_cb        = (OpenCb)open_module_cb;
			module_info->read_cb        = (ReadCb)read_module_cb;
			module_info->write_cb       = (WriteCb)write_module_cb;
			module_info->get_io_stat_cb = (GetStatCb)get_io_stat_module_cb;
			module_info->args_for_stat = module_args;
		}

		if(info.segments[1].memsz != 0){

			module_name = ksceKernelAllocHeapMemory(heap_uid, 0x20);
			module_args = ksceKernelAllocHeapMemory(heap_uid, sizeof(ModuleStat));
			module_info = ksceKernelAllocHeapMemory(heap_uid, sizeof(FileEntry));

			(*ppEntry) = module_info;
			ppEntry = &module_info->next;
			memset(module_info, 0, sizeof(FileEntry));

			memcpy(module_name, info.module_name, 0x1F);

			module_args->name = module_name;
			module_args->seg = 1;

			module_info->next     = NULL;
			module_info->next_dir = NULL;
			module_info->open_allow_flags = SCE_O_RDONLY | SCE_O_WRONLY;
			module_info->flags    = 0;

			module_name = ksceKernelAllocHeapMemory(heap_uid, 0x28);
			snprintf(module_name, 0x27, "%s data", module_args->name);
			module_info->name     = module_name;
			module_info->size     = (SceOff)info.segments[1].memsz;

			module_info->stat = ksceKernelAllocHeapMemory(heap_uid, sizeof(SceIoStat));

			setToFileEntryTplStat(module_info);
			module_info->stat->st_size = (SceOff)info.segments[1].memsz;

			module_info->open_cb        = (OpenCb)open_module_cb;
			module_info->read_cb        = (ReadCb)read_module_cb;
			module_info->write_cb       = (WriteCb)write_module_cb;
			module_info->get_io_stat_cb = (GetStatCb)get_io_stat_module_cb;
			module_info->args_for_stat = module_args;
		}
	}

	return 0;
}

int init_module_kernel(void){

	FileEntry *entry;

	createFileEntry("kernel modules", &entry);
	setToDirEntry(entry);

	entry->dopen_cb = (DopenCb)dopen_cb_module;

	return 0;
}
