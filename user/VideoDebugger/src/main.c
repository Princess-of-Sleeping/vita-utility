
#include <psp2/kernel/modulemgr.h>
#include <psp2/kernel/clib.h>
#include <taihen.h>


#define HookImport(module_name, library_nid, func_nid, func_name) taiHookFunctionImport(&func_name ## _ref, module_name, library_nid, func_nid, func_name ## _patch)


tai_hook_ref_t sceRegMgrGetKeyInt_ref;
int sceRegMgrGetKeyInt_patch(const char *category, const char *name, int *pValue){

	int res;

	res = TAI_CONTINUE(int, sceRegMgrGetKeyInt_ref, category, name, pValue);

	if(pValue != NULL && sceClibStrcmp(category, "/CONFIG/VIDEO/") == 0 && sceClibStrcmp(name, "debug_videoplayer") == 0){
		*pValue = 1;
		res = 0;
	}

	return res;
}

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp){

	HookImport("SceVideoPlayer", 0xC436F916, 0x16DDF3DC, sceRegMgrGetKeyInt);

	return SCE_KERNEL_START_SUCCESS;
}
