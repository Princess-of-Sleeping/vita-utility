/*
 * SceUsbstorVstorFat16
 * Copyright (C) 2021 Princess of Sleeping
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/threadmgr.h>
#include <psp2kern/kernel/sysmem.h>
#include <psp2kern/kernel/sysclib.h>
#include <psp2kern/io/fcntl.h>
#include <psp2/usbstorvstor.h>
#include <taihen.h>

typedef struct SceUsbstorContext { // size is 0x158
	int data_0x00[3];
	SceUID fd;
	SceUInt32 nSector;
	int data_0x14[3];
	int data_0x20[4];
	int data_0x30[2];
	SceUID evf_id;
	SceUID usbstorVstor_memid;
	SceUID usbstorExt_memid;
	void  *usbstorExt_base;
	int    data_0x48;
	void  *usbstorVstor_base;
	int    data_0x50;
	int    usb_type;

	// 0x58
	char path[0x100];
} SceUsbstorContext;

#define HookOffset(modid, offset, thumb, func_name) taiHookFunctionOffsetForKernel(0x10005, &func_name ## _ref, modid, 0, offset, thumb, func_name ## _patched)

#define HookRelease(hook_uid, hook_func_name)({ \
	(hook_uid > 0) ? taiHookReleaseForKernel(hook_uid, hook_func_name ## _ref) : -1; \
})

/*
 * 0x81001738 	35 E0 	B #0x810017A6
 */
const char path_check_patch[] = {
	0x35, 0xE0
};

int module_get_offset(SceUID pid, SceUID modid, int segidx, uint32_t offset, uintptr_t *dst);

char mbr[0x200] __attribute__((aligned(0x40)));
SceUsbstorContext *pUsbstorContext;
int (* device_init)(void);

int device_init_patch(void){

	int res = device_init();

	if(res == 0x80244112 && pUsbstorContext->usb_type == SCE_USBSTOR_VSTOR_TYPE_FAT){

		memset(mbr, 0x41, sizeof(mbr));

		ksceIoLseek(pUsbstorContext->fd, 0LL, SCE_SEEK_SET);
		ksceIoRead(pUsbstorContext->fd, mbr, sizeof(mbr));

		if(strncmp(&mbr[0x36], "FAT16", 5) != 0)
			return res;

		if(*(uint16_t *)(&mbr[0x13]) != 0){
			pUsbstorContext->nSector = *(uint16_t *)(&mbr[0x13]);
		}else if(*(uint32_t *)(&mbr[0x20]) != 0){
			pUsbstorContext->nSector = *(uint32_t *)(&mbr[0x20]);
		}else{
			return res;
		}

		SceKernelAllocMemBlockKernelOpt opt;
		memset(&opt, 0, sizeof(opt));
		opt.size = sizeof(opt);
		opt.attr = 0x200000;

		SceUID memid = ksceKernelAllocMemBlock("SceUsbstorExt", 0x10208006, 0x20000, &opt);

		void *base;
		ksceKernelGetMemBlockBase(memid, &base);

		pUsbstorContext->usbstorExt_memid = memid;
		pUsbstorContext->usbstorExt_base  = base;

		ksceKernelSetEventFlag(pUsbstorContext->evf_id, 0x200);
		res = 0;
	}

	return res;
}

void _start() __attribute__ ((weak, alias ("module_start")));
int module_start(SceSize argc, const void *args){

	SceUID moduleid = ksceKernelSearchModuleByName("SceUsbstorVStorDriver");

	// A patch that allows reading img from paths other than "ux0:/umass/"
	taiInjectDataForKernel(0x10005, moduleid, 0, 0x1738, &path_check_patch, 0x2);

	void *p;

	module_get_offset(0x10005, moduleid, 1, 0, (uintptr_t *)&pUsbstorContext);
	module_get_offset(0x10005, moduleid, 1, 0x158 + 0xC, (uintptr_t *)&p);

	device_init = *(void **)(p);
	*(void **)(p) = device_init_patch;

	return SCE_KERNEL_START_SUCCESS;
}
