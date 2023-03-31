
#include <psp2/kernel/modulemgr.h>
#include <psp2/kernel/clib.h>
#include <taihen.h>


#define HookImport(module_name, library_nid, func_nid, func_name) taiHookFunctionImport(&func_name ## _ref, module_name, library_nid, func_nid, func_name ## _patch)


tai_hook_ref_t _sceNpDrmCheckDrmReset_ref;
int _sceNpDrmCheckDrmReset_patch(void *a1, int a2, int *option){

	int res;
	SceUInt8 param[0x40];

	sceClibMemcpy(param, a1, option[3]);

	param[1] |= 0xC0;

	res = TAI_CONTINUE(int, _sceNpDrmCheckDrmReset_ref, param, a2, option);

	return res;
}

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp){

	HookImport("SceShell", 0xF2799B1B, 0x4458812B, _sceNpDrmCheckDrmReset);

	return SCE_KERNEL_START_SUCCESS;
}
