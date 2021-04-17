/*
 * PS Vita Mac address spoofer
 * Copyright (C) 2021 Princess of Sleeping
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/threadmgr.h>
#include <psp2kern/kernel/sysclib.h>
#include <psp2kern/io/fcntl.h>
#include <taihen.h>

#define HookExport(module_name, library_nid, func_nid, func_name) \
	taiHookFunctionExportForKernel(0x10005, &func_name ## _ref, module_name, library_nid, func_nid, func_name ## _patch)

int module_get_offset(SceUID pid, SceUID modid, int segidx, uint32_t offset, uintptr_t *dst);

typedef struct SceNetPsDeviceConfig { // maybe size is variable, max 0x5DC?
	void *data_0x00;
	struct SceNetPsDeviceConfig *next;
	void *data_0x08;
	void *data_0x0C;
	void *data_0x10;
	char device[0x10];
	int data_0x24;
	int data_0x28;
	int data_0x2C;
	int data_0x30;
	int data_0x34;
	uint16_t data_0x38;
	uint8_t  data_0x3A;
	uint8_t  data_0x3B;

	// more...
} SceNetPsDeviceConfig;

char mac_address[6];

int clear_netps_mac_address(void){

	SceUID moduleid = ksceKernelSearchModuleByName("SceNetPs");

	SceNetPsDeviceConfig *pDeviceConfig = NULL;

	// From 3.60 to 3.68, common offset.
	module_get_offset(0x10005, moduleid, 1, 0x8102EC20 - 0x8102E000, (uintptr_t *)&pDeviceConfig);

	pDeviceConfig = *(SceNetPsDeviceConfig **)pDeviceConfig;

	while(pDeviceConfig != NULL){
		if(pDeviceConfig->data_0x38 != 0x17 && pDeviceConfig->data_0x38 != 0x18 && strncmp(pDeviceConfig->device, "wlan", 4) == 0){
			memcpy((*(void **)pDeviceConfig->data_0x00) + 0x5C, mac_address, 6);
			break;
		}

		pDeviceConfig = pDeviceConfig->next;
	}

	return 0;
}

tai_hook_ref_t SceNetPsForDriver_1ABF937D_ref;
int SceNetPsForDriver_1ABF937D_patch(void *pConfig){

	if(pConfig != NULL)
		memcpy(pConfig + 0x5C, mac_address, 6);

	return TAI_CONTINUE(int, SceNetPsForDriver_1ABF937D_ref, pConfig);
}

int load_mac_address(void *dst, SceSize size){

	SceUID fd;

	fd = ksceIoOpen("host0:data/spoof/mac_address.bin", SCE_O_RDONLY, 0);
	if(fd < 0)
		fd = ksceIoOpen("sd0:data/spoof/mac_address.bin", SCE_O_RDONLY, 0);
	if(fd < 0)
		fd = ksceIoOpen("ux0:data/spoof/mac_address.bin", SCE_O_RDONLY, 0);
	if(fd < 0)
		return fd;

	ksceIoRead(fd, dst, size);
	ksceIoClose(fd);

	return 0;
}

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp){

	int res;

	res = load_mac_address(mac_address, sizeof(mac_address));
	if(res < 0)
		memset(mac_address, 0xA5, sizeof(mac_address));

	if(ksceKernelSearchModuleByName("SceWlanBt") >= 0){
		// Add delay only boot time.
		if(ksceKernelSearchModuleByName("SceSysStateMgr") >= 0){
			// Wlan/Bt initialization takes a considerable amount of time, so you have to wait for a while.
			ksceKernelDelayThread(2 * 1000 * 1000);
		}

		clear_netps_mac_address();
		return SCE_KERNEL_START_NO_RESIDENT;
	}

	HookExport("SceNetPs", 0xB2A5C920, 0x1ABF937D, SceNetPsForDriver_1ABF937D);

	return SCE_KERNEL_START_SUCCESS;
}
