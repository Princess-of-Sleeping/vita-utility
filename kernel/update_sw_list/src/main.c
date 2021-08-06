/*
 * SceSblUpdateMgr test
 * Copyright (C) 2021 Princess of Sleeping
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/sysclib.h>
#include <psp2kern/kernel/debug.h>

typedef struct SceSblUsSwPscode { // size is 0xC-bytes
	SceUInt16 product_code_upper; // or lower
	SceUInt16 product_code_lower; // or upper
	SceUInt16 product_sub_code;
	SceUInt16 data_0x06;
	SceUInt32 number;
} SceSblUsSwPscode;

typedef struct SceSblUsSwFw { // size is 0xC-bytes
	SceUInt32 fw1;
	SceUInt32 fw2;
	SceInt32 flags;
} SceSblUsSwFw;

int update_sw_list_test(void){

	SceUID module_id;

	module_id = ksceKernelSearchModuleByName("SceSblUpdateMgr");
	if(module_id < 0){
		ksceDebugPrintf("%s:Target module not found : 0x%X\n", __FUNCTION__, module_id);
		return module_id;
	}

	SceKernelModuleInfo module_info;
	memset(&module_info, 0, sizeof(module_info));
	module_info.size = sizeof(module_info);

	ksceKernelGetModuleInfo(0x10005, module_id, &module_info);

	/*
	 * If you have IsDiag, min fw will be set to 0xFFFFFFFF regardless of this list
	 * offset based 3.60
	 */
	const SceSblUsSwFw     *pUsSwFw     = (const SceSblUsSwFw     *)(module_info.segments[0].vaddr + 0xCF8C);
	const SceSblUsSwPscode *pUsSwPscode = (const SceSblUsSwPscode *)(module_info.segments[0].vaddr + 0xD3C4);

	for(int i=0;i<0x59;i++){
		ksceDebugPrintf(
			"%03d: product_code_upper=0x%X product_code_lower=0x%X product_sub_code=0x%03X fw1=0x%08X fw2=0x%08X flags=0x%08X\n",
			i,
			pUsSwPscode[i].product_code_upper,
			pUsSwPscode[i].product_code_lower,
			pUsSwPscode[i].product_sub_code,
			pUsSwFw[pUsSwPscode[i].number].fw1,
			pUsSwFw[pUsSwPscode[i].number].fw2,
			pUsSwFw[pUsSwPscode[i].number].flags
		);
	}

	return 0;
}

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp){

	update_sw_list_test();

	return SCE_KERNEL_START_NO_RESIDENT;
}
