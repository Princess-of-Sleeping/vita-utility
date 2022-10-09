/*
 * Kernel panic debugger
 * Copyright (C) 2021 Princess of Sleeping
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/debug.h>
#include <psp2kern/kernel/excpmgr.h>
#include <taihen.h>

#define HookOffset(modid, offset, thumb, func_name) taiHookFunctionOffsetForKernel(SCE_GUID_KERNEL_PROCESS_ID, &func_name ## _ref, modid, 0, offset, thumb, func_name ## _patch)

#define VECTOR_NUMBER(v) (sizeof(v) / sizeof(v[0]))

SceUID hook_id[3];

const char * const cpu_reg_name[0x10] = {
	"r0", "r1", "r2", "r3",
	"r4", "r5", "r6", "r7",
	"r8", "r9", "r10", "r11",
	"ip", "sp", "lr", "pc",
};

const char * const dbg_event_string_list[] = {
	"Halt Request debug event",
	"Breakpoint debug event",
	"Asynchronous Watchpoint debug event",
	"BKPT Instruction debug event",

	"External Debug Request debug event",
	"Vector Catch debug event",
	"0x6",
	"0x7",

	"OS Unlock Catch debug event",
	"0x9",
	"Synchronous Watchpoint debug event",
	"0xB",

	"0xC",
	"0xD",
	"0xE",
	"0xF"
};

int print_excp_info(const SceExcpmgrExceptionContext *context, SceUInt32 pc, SceUInt32 cpsr){

	ksceDebugPrintf("\n**** Exception info ****\n\n");

	ksceDebugPrintf("%-02s-%-03s : 0x%08X 0x%08X 0x%08X 0x%08X\n", cpu_reg_name[0], cpu_reg_name[3], context->r0, context->r1, context->r2, context->r3);
	ksceDebugPrintf("%-02s-%-03s : 0x%08X 0x%08X 0x%08X 0x%08X\n", cpu_reg_name[4], cpu_reg_name[7], context->r4, context->r5, context->r6, context->r7);
	ksceDebugPrintf("%-02s-%-03s : 0x%08X 0x%08X 0x%08X 0x%08X\n", cpu_reg_name[8], cpu_reg_name[11], context->r8, context->r9, context->r10, context->r11);
	ksceDebugPrintf("%s     : 0x%08X\n", cpu_reg_name[12], context->r12);
	ksceDebugPrintf("%s     : 0x%08X\n", cpu_reg_name[13], context->sp);
	ksceDebugPrintf("%s     : 0x%08X\n", cpu_reg_name[14], context->lr);
	ksceDebugPrintf("%s     : 0x%08X\n", cpu_reg_name[15], context->address_of_faulting_instruction);
	ksceDebugPrintf("CPSR   : 0x%08X\n", context->SPSR);
	ksceDebugPrintf("\n");

	for(int i=0;i<8;i++){
		ksceDebugPrintf(
			"d%-02d-d%-02d : 0x%016llX 0x%016llX 0x%016llX 0x%016llX\n",
			i * 4, i * 4 + 3,
			context->VFP_registers[i + 0],
			context->VFP_registers[i + 1],
			context->VFP_registers[i + 2],
			context->VFP_registers[i + 3]
		);
	}

	ksceDebugPrintf("FPSCR : 0x%08X\n", context->FPSCR);
	ksceDebugPrintf("FPEXC : 0x%08X\n", context->FPEXC);
	ksceDebugPrintf("\n");

	ksceDebugPrintf("CONTEXTIDR : 0x%08X\n", context->CONTEXTIDR);
	ksceDebugPrintf("TPIDRURW   : 0x%08X\n", context->TPIDRURW);
	ksceDebugPrintf("TPIDRURO   : 0x%08X\n", context->TPIDRURO);
	ksceDebugPrintf("TPIDRPRW   : 0x%08X\n", context->TPIDRPRW);
	ksceDebugPrintf("TTBR1      : 0x%08X\n", context->TTBR1);
	ksceDebugPrintf("DACR       : 0x%08X\n", context->DACR);
	ksceDebugPrintf("\n");

	ksceDebugPrintf("DFSR : 0x%08X\n", context->DFSR);
	ksceDebugPrintf("IFSR : 0x%08X\n", context->IFSR);
	ksceDebugPrintf("DFAR : 0x%08X\n", context->DFAR);
	ksceDebugPrintf("IFAR : 0x%08X\n", context->IFAR);

	if((context->IFSR & 0x140F) == 2){
		ksceDebugPrintf(
			"DBGDSCR : 0x%08X [ %s ]\n",
			context->DBGSCRext,
			dbg_event_string_list[(context->DBGSCRext >> 2) & 0xF]
		);
	}

/*
	ksceDebugPrintf("PAR        0x%08X\n", context->PAR);
	ksceDebugPrintf("TEEHBR     0x%08X\n", context->TEEHBR);
*/

	return 0;
}

tai_hook_ref_t sceSDfMgrDbgExcpPabtHandler_ref;
int sceSDfMgrDbgExcpPabtHandler_patch(SceExcpmgrExceptionContext *context, SceUInt32 pc, SceUInt32 cpsr){

	int res;

	print_excp_info(context, pc, cpsr);

	res = TAI_CONTINUE(int, sceSDfMgrDbgExcpPabtHandler_ref, context, pc, cpsr);

	// Maybe won't come back here

	return res;
}

tai_hook_ref_t sceSDfMgrDbgExcpDabtHandler_ref;
int sceSDfMgrDbgExcpDabtHandler_patch(SceExcpmgrExceptionContext *context, SceUInt32 pc, SceUInt32 cpsr){

	int res;

	print_excp_info(context, pc, cpsr);

	res = TAI_CONTINUE(int, sceSDfMgrDbgExcpDabtHandler_ref, context, pc, cpsr);

	// Maybe won't come back here

	return res;
}

tai_hook_ref_t sceSDfMgrDbgExcpUndefHandler_ref;
int sceSDfMgrDbgExcpUndefHandler_patch(SceExcpmgrExceptionContext *context, SceUInt32 pc, SceUInt32 cpsr){

	int res;

	print_excp_info(context, pc, cpsr);

	res = TAI_CONTINUE(int, sceSDfMgrDbgExcpUndefHandler_ref, context, pc, cpsr);

	// Maybe won't come back here

	return res;
}

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp){

	int res;
	tai_module_info_t tai_module_info;

	res = ksceKernelSearchModuleByName("SceDeci4pSDfMgr");
	if(res < 0)
		return SCE_KERNEL_START_NO_RESIDENT;

	tai_module_info.size = sizeof(tai_module_info);

	res = taiGetModuleInfoForKernel(SCE_GUID_KERNEL_PROCESS_ID, "SceDeci4pSDfMgr", &tai_module_info);
	if(res < 0)
		return SCE_KERNEL_START_NO_RESIDENT;

	switch(tai_module_info.module_nid){
	case 0xC7A13F58: // 3.60
		hook_id[0] = HookOffset(tai_module_info.modid, 0x2234, 1, sceSDfMgrDbgExcpPabtHandler);
		hook_id[1] = HookOffset(tai_module_info.modid, 0x22F0, 1, sceSDfMgrDbgExcpDabtHandler);
		hook_id[2] = HookOffset(tai_module_info.modid, 0x23AC, 1, sceSDfMgrDbgExcpUndefHandler);
	break;
	default:
		ksceDebugPrintf("Unknown module fingerprint=0x%08X\n", tai_module_info.module_nid);
		return SCE_KERNEL_START_NO_RESIDENT;
	}

	for(int i=0;i<VECTOR_NUMBER(hook_id);i++){
		if(hook_id[i] < 0){
			ksceDebugPrintf("Failed hook(%d) = 0x%X\n", i, hook_id[i]);
		}
	}

	return SCE_KERNEL_START_SUCCESS;
}
