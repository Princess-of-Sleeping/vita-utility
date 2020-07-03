/*
 * SceVfs mount PoC
 * Copyright (C) 2020 Princess of Sleeping
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/sysmem.h>

typedef struct SceVfsMount2 { // size is 0x14
	const char *unit;     // ex:"host0:"
	const char *device1;  // ex:"bsod_dummy_host_fs"
	const char *device2;  // ex:"bsod_dummy_host_fs"
	int data_0x0C;        // ex:0
	int data_0x10;        // ex:0
} SceVfsMount2;

typedef struct SceVfsMount {   // size is 0x20
	const char *device;    // ex:"/host"
	int data_0x04;
	int data_0x08;         // ex:0x03000004
	int data_0x0C;         // ex:0x00008006
	const char *data_0x10; // ex:"bsod_dummy_host_fs"
	int data_0x14;
	const SceVfsMount2 *data_0x18;
	int data_0x1C;
} SceVfsMount;

typedef struct SceVfsUmount {
	const char *device; // ex:"/host"
	int data_0x04;
} SceVfsUmount;

typedef struct SceVfsTable { // size is 0x34?
	const void *func00;
	const void *func04;
	const void *func08;
	const void *func0C;
	const void *func10;
	const void *func14;
	const void *func18;
	const void *func1C;
	const void *func20;
	const void *func24;
	const void *func28;
	const void *func2C;
	const void *func30;
} SceVfsTable;

typedef struct SceVfsTable2 { // size is 0x74?
	const void *func00;
	const void *func04;
	const void *func08;
	const void *func0C;
	const void *func10;
	const void *func14;
	const void *func18;
	const void *func1C;
	const void *func20;
	const void *func24;
	const void *func28;
	const void *func2C;
	const void *func30;
	const void *func34;
	const void *func38;
	const void *func3C;
	const void *func40;
	const void *func44;
	const void *func48;
	const void *func4C;
	const void *func50;
	const void *func54;
	const void *func58;
	const void *func5C;
	const void *func60;
	const void *func64;
	const void *func68;
	const void *func6C;
	const void *func70;
} SceVfsTable2;

typedef struct SceVfsAdd {     // size is 0x1C
	const SceVfsTable *func_ptr1;
	const char *device;    // ex:"bsod_dummy_host_fs"
	int data_0x08;
	int data_0x0C;
	int data_0x10;         // ex:0x10
	const SceVfsTable2 *func_ptr2;
	int data_0x18;
} SceVfsAdd;

int ksceVfsMount(const SceVfsMount *pVfsMount);
int ksceVfsUnmount(const SceVfsUmount *pVfsUmount);

int ksceVfsAddVfs(SceVfsAdd *pVfsAdd);
int ksceVfsDeleteVfs(const char *fs); // "deci4p_drfp_dev_fs"

const SceVfsMount2 vfs_mount2 = {
	.unit      = "host0:",
	.device1   = "faps_dummy_host_fs",
	.device2   = "faps_dummy_host_fs",
	.data_0x0C = 0,
	.data_0x10 = 0
};

const SceVfsMount vfs_mount = {
	.device    = "/host",
	.data_0x04 = 0,
	.data_0x08 = 0x03000004,
	.data_0x0C = 0x00008006,
	.data_0x10 = "faps_dummy_host_fs",
	.data_0x14 = 0,
	.data_0x18 = &vfs_mount2,
	.data_0x1C = 0
};

const SceVfsUmount vfs_umount = {
	.device = "/host",
	.data_0x04 = 0
};

SceVfsAdd vfs_add;

void sub_81002110(const char *path){
	ksceDebugPrintf("%s\n", path);
}

int sub_81001DEC(void *a1){

	*(uint32_t *)(*(uint32_t *)(a1) + 0x68) = 0x80;
	*(uint32_t *)(*(uint32_t *)(a1) + 0xC0) = 0x10000;

	return 0;
}

int sub_81001DFE(void *a1){
	return 0;
}

int sub_81001E02(void *a1){

	*(uint32_t *)(*(uint32_t *)(a1 + 8) + 0x48) = *(uint32_t *)(*(uint32_t *)(a1) + 0xC4);
	*(uint32_t *)(*(uint32_t *)(a1 + 8) + 0x4C) = *(uint32_t *)(a1);
	*(uint32_t *)(*(uint32_t *)(a1 + 8) + 0x50) = 0;
	*(uint32_t *)(*(uint32_t *)(a1 + 8) + 0x60) = 0;
	*(uint32_t *)(*(uint32_t *)(a1 + 8) + 0x64) = 0;
	*(uint32_t *)(*(uint32_t *)(a1 + 8) + 0x74) = 1;
	*(uint32_t *)(*(uint32_t *)(a1 + 8) + 0x78) = 0x1002;
	*(uint32_t *)(*(uint32_t *)(a1 + 8) + 0x80) = 0;
	*(uint32_t *)(*(uint32_t *)(a1 + 8) + 0x84) = 0;
	*(uint32_t *)(*(uint32_t *)(a1 + 8) + 0x88) = 0;
	*(uint32_t *)(*(uint32_t *)(a1 + 8) + 0x8C) = 0;

	return 0;
}

int sub_81001E2E(void *a1){
	return 0;
}

int sub_81001DE8(void *a1){
	return 0;
}

int sub_81001E32(void *a1){
	return 0x80010086;
}

typedef struct SceVfsPathElem {     // size is 0x1C
	void *data_0x00;
	char *path;
	void *data_0x08;
	void *data_0x0C;
	void *data_0x10;
	int data_0x14;
	void *data_0x18;
} SceVfsPathElem;

// call from ksceVfsOpDecodePathElem

int sub_8100222C(SceVfsPathElem *a1){

	void *lr;
	__asm__ volatile ("mov %0, lr" : "=r" (lr));

	ksceDebugPrintf("sub_8100222C:0x%X\n", lr);
	sub_81002110(a1->path);

	return 0;
}

int sub_8100221A(void *a1){
	ksceDebugPrintf("sub_8100221A\n");
	sub_81002110((const char *)*(uint32_t *)(*(uint32_t *)(a1 + 4)));
	return 0x80010086;
}

int sub_81001E42(void *a1){
	ksceDebugPrintf("sub_81001E42\n");
	return 0x80010086;
}

int sub_81001E4A(void *a1){
	ksceDebugPrintf("sub_81001E4A\n");
	return 0x80010086;
}

/*
 * io_get_stat?
 */
int sub_81001E3A(void *a1){
	ksceDebugPrintf("sub_81001E3A\n");
	return 0x80010086;
}

int sub_81001E52(void *a1){
	ksceDebugPrintf("sub_81001E52\n");
	return 0x80010086;
}

int sub_81001E5A(void *a1){
	ksceDebugPrintf("sub_81001E5A\n");
	return 0x80010086;
}

/*
 * maybe lseek
 */
uint64_t sub_810020C8(void *a1){
	ksceDebugPrintf("sub_810020C8\n");
	return 0xFFFFFFFF80010086;
}

int sub_81002208(void *a1){
	ksceDebugPrintf("sub_81002208\n");
	sub_81002110((const char *)*(uint32_t *)(*(uint32_t *)(a1 + 8)));
	return 0x80010086;
}

int sub_810021F6(void *a1){
	ksceDebugPrintf("sub_810021F6\n");
	sub_81002110((const char *)*(uint32_t *)(*(uint32_t *)(a1 + 8)));
	return 0x80010086;
}

int sub_810021E4(void *a1){
	ksceDebugPrintf("sub_810021E4\n");
	sub_81002110((const char *)*(uint32_t *)(*(uint32_t *)(a1 + 8)));
	return 0x80010086;
}

int sub_810021D2(void *a1){
	ksceDebugPrintf("sub_810021D2\n");
	sub_81002110((const char *)*(uint32_t *)(*(uint32_t *)(a1 + 4)));
	return 0x80010086;
}

int sub_810020D8(void *a1){
	ksceDebugPrintf("sub_810020D8\n");
	return 0x80010086;
}

int sub_810020E0(void *a1){
	ksceDebugPrintf("sub_810020E0\n");
	return 0x80010086;
}

/*
 * io_dopen?
 */
int sub_810021C0(void *a1){
	ksceDebugPrintf("sub_810021C0\n");
	sub_81002110((const char *)*(uint32_t *)(*(uint32_t *)(a1 + 4)));
	return 0x80010086;
}

int sub_810021AE(void *a1){
	ksceDebugPrintf("sub_810021AE\n");
	sub_81002110((const char *)*(uint32_t *)(*(uint32_t *)(a1 + 4)));
	return 0x80010086;
}

int sub_8100219C(void *a1){
	ksceDebugPrintf("sub_8100219C\n");
	sub_81002110((const char *)*(uint32_t *)(*(uint32_t *)(a1 + 4)));
	return 0x80010086;
}

int sub_810020F8(void *a1){
	ksceDebugPrintf("sub_810020F8\n");
	return 0x80010086;
}

int sub_81002100(void *a1){
	ksceDebugPrintf("sub_81002100\n");
	return 0x80010086;
}

int sub_81002108(void *a1){
	ksceDebugPrintf("sub_81002108\n");
	return 0;
}

int sub_8100210C(void *a1){
	ksceDebugPrintf("sub_8100210C\n");
	return 0;
}

int sub_810020E8(void *a1){
	ksceDebugPrintf("sub_810020E8\n");
	return 0x80010086;
}

int sub_810020F0(void *a1){
	ksceDebugPrintf("sub_810020F0\n");
	return 0x80010086;
}

const SceVfsTable vfs_table = {
	.func00 = sub_81001DEC,
	.func04 = sub_81001DFE,
	.func08 = sub_81001E02,
	.func0C = NULL,
	.func10 = NULL,
	.func14 = NULL,
	.func18 = sub_81001E2E,
	.func1C = NULL,
	.func20 = sub_81001DE8,
	.func24 = NULL,
	.func28 = NULL,
	.func2C = sub_81001E32,
	.func30 = sub_8100222C
};

const SceVfsTable2 vfs_table2 = {
	.func00 = sub_8100221A,
	.func04 = sub_81001E42,
	.func08 = sub_81001E4A,
	.func0C = sub_81001E3A,
	.func10 = sub_81001E52,
	.func14 = sub_81001E5A,
	.func18 = sub_810020C8,
	.func1C = NULL,
	.func20 = sub_81002208,
	.func24 = sub_810021F6,
	.func28 = sub_810021E4,
	.func2C = sub_810021D2,
	.func30 = sub_810020D8,
	.func34 = sub_810020E0,
	.func38 = sub_810021C0,
	.func3C = sub_810021AE,
	.func40 = sub_8100219C,
	.func44 = NULL,
	.func48 = sub_810020F8,
	.func4C = sub_81002100,
	.func50 = sub_81002108,
	.func54 = NULL,
	.func58 = NULL,
	.func5C = sub_8100210C,
	.func60 = sub_810020E8,
	.func64 = sub_810020F0,
	.func64 = NULL,
	.func68 = NULL,
	.func70 = NULL
};

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp){

	ksceVfsUnmount(&vfs_umount);

	ksceVfsDeleteVfs("bsod_dummy_host_fs");

	vfs_add.func_ptr1 = &vfs_table;
	vfs_add.device    = "faps_dummy_host_fs";
	vfs_add.data_0x08 = 0;
	vfs_add.data_0x0C = 0;
	vfs_add.data_0x10 = 0x10;
	vfs_add.func_ptr2 = &vfs_table2;
	vfs_add.data_0x18 = 0;

	ksceVfsAddVfs(&vfs_add);

	ksceVfsMount(&vfs_mount);

	return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize args, void *argp){
	return SCE_KERNEL_STOP_CANCEL;
}
