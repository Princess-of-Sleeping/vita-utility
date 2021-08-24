/*
 * Mapping userland memory block by any paddr
 * Copyright (C) 2021 Princess of Sleeping
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/proc_event.h>
#include <psp2kern/kernel/sysclib.h>
#include <psp2kern/kernel/sysmem.h>
#include <psp2kern/kernel/debug.h>

SceUID proc_event_id = -1, memid = -1;
void *base;

// should be array count <= 0x20 for one mem block
// if set count >= 0x21, system get stuck
SceKernelAddrPair output[0x20];

int create_proc(SceUID pid, SceProcEventInvokeParam2 *a2, int a3){

	SceKernelAddrPair input;

	input.addr   = (uintptr_t)base;
	input.length = 0x1000;

	SceKernelPaddrList PA_list;

	PA_list.size       = sizeof(PA_list);
	PA_list.list_size  = 0x20;
	PA_list.ret_length = 0;
	PA_list.ret_count  = 0;
	PA_list.list       = output;

	int res = ksceKernelGetPaddrList(&input, &PA_list);

	ksceDebugPrintf("sceKernelGetPaddrList : 0x%X\n", res);
	ksceDebugPrintf("        ret_length : 0x%X\n", PA_list.ret_length);
	ksceDebugPrintf("        ret_count  : 0x%X\n", PA_list.ret_count);

	if(0){
		output[0].addr   = 0x1F840000;
		output[0].length = 0x1000;
	}

	// set fake paddr for test
	for(int i=1;i<0x20;i++){
		output[i].addr   = output[0].addr;
		output[i].length = output[0].length;
	}

	PA_list.ret_length = 0x20;
	PA_list.ret_count  = 0x20;


	// Start mapping to userland
	SceKernelAllocMemBlockKernelOpt opt;
	memset(&opt, 0, sizeof(opt));

	opt.size       = sizeof(opt);
	opt.attr       = SCE_KERNEL_ALLOC_MEMBLOCK_ATTR_HAS_PID | SCE_KERNEL_ALLOC_MEMBLOCK_ATTR_HAS_PADDR_LIST;
	opt.pid        = pid;
	opt.paddr_list = &PA_list;

	SceUID memid = ksceKernelAllocMemBlock("SceUserMemoryMap", SCE_KERNEL_MEMBLOCK_TYPE_USER_RW, 0x20000, &opt);

	void *user_base;
	ksceKernelGetMemBlockBase(memid, &user_base);

	ksceKernelCreateUserUid(pid, memid);

	ksceDebugPrintf("user_base : 0x%X\n", user_base);

	return 0;
}

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp){

	int res;

	SceProcEventHandler handler;
	memset(&handler, 0, sizeof(handler));
	handler.size = sizeof(handler);
	handler.create = create_proc;

	res = ksceKernelRegisterProcEventHandler("SceMemoryMappingByPaddrTest", &handler, 0);
	if(res < 0){
		goto error;
	}

	proc_event_id = res;


	res = ksceKernelAllocMemBlock("SceKernelMemoryMap", SCE_KERNEL_MEMBLOCK_TYPE_KERNEL_RW, 0x1000, NULL);
	if(res < 0){
		goto error;
	}

	memid = res;

	res = ksceKernelGetMemBlockBase(memid, &base);
	if(res < 0){
		goto error;
	}

	for(int i=0;i<0x10;i++)
		memset(base + (i * 0x100), 0x11 * i, 0x100);

	return SCE_KERNEL_START_SUCCESS;

error:
	if(memid >= 0){
		ksceKernelFreeMemBlock(memid);
		memid = -1;
	}

	if(proc_event_id >= 0){
		ksceKernelUnregisterProcEventHandler(proc_event_id);
		proc_event_id = -1;
	}

	return SCE_KERNEL_START_FAILED;
}
