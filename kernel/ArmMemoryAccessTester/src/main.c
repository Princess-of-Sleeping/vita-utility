/*
 * ARM memory access tester
 * Copyright (C) 2021, Princess of Sleeping
 */

#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/sysmem.h>
#include <psp2kern/kernel/proc_event.h>
#include <taihen.h>

int module_get_offset(SceUID pid, SceUID modid, int segidx, size_t offset, uintptr_t *addr);

int (* sub_810233B8_get_current_ctx)(SceKernelProcessContext *ctx);
int (* sub_810233D8_set_context)(SceKernelProcessContext *ctx);

int (* sub_8101FF20_get_process_ctx)(SceUID pid, void **as_ctx);
int (* sub_8100CC80_set_context)(void *as_ctx);

int process_create(SceUID pid, SceProcEventInvokeParam2 *a2, int a3){

	ksceDebugPrintf("AAAAAA\n");

	int a1 = ksceKernelCpuDisableInterrupts();

	void *as_ctx;
	SceKernelProcessContext ctx;

	sub_810233B8_get_current_ctx(&ctx);
	sub_8101FF20_get_process_ctx(pid, &as_ctx);
	sub_8100CC80_set_context(as_ctx); // Settings to TTBR1, etc

	/*
	 * Accessible userland memory directly.
	 * But I don't know if this is based on ARM specifications.
	 */
	uint32_t value = *(uint32_t *)(0x81000000);

	sub_810233D8_set_context(&ctx);

	ksceKernelCpuEnableInterrupts(a1);

	ksceDebugPrintf("value:0x%08X\n", value);

	return 0;
}

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp){

	SceProcEventHandler handler;
	memset(&handler, 0, sizeof(handler));
	handler.size   = sizeof(handler);
	handler.create = process_create;

	ksceKernelRegisterProcEventHandler("SceMemoryProcEvent", &handler, 0);

	SceUID module_id = ksceKernelSearchModuleByName("SceSysmem");

	// Offset bases 3.60
	module_get_offset(SCE_GUID_KERNEL_PROCESS_ID, module_id, 0, 0x233B8 | 1, (uintptr_t *)&sub_810233B8_get_current_ctx);
	module_get_offset(SCE_GUID_KERNEL_PROCESS_ID, module_id, 0, 0x233D8 | 1, (uintptr_t *)&sub_810233D8_set_context);
	module_get_offset(SCE_GUID_KERNEL_PROCESS_ID, module_id, 0, 0x1FF20 | 1, (uintptr_t *)&sub_8101FF20_get_process_ctx);
	module_get_offset(SCE_GUID_KERNEL_PROCESS_ID, module_id, 0,  0xCC80 | 1, (uintptr_t *)&sub_8100CC80_set_context);

	return SCE_KERNEL_START_SUCCESS;
}
