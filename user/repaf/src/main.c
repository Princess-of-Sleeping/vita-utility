
#include <psp2/kernel/modulemgr.h>
#include <psp2/sysmodule.h>
#include <taihen.h>


#define HookImport(module_name, library_nid, func_nid, func_name) taiHookFunctionImport(&func_name ## _ref, module_name, library_nid, func_nid, func_name ## _patch)


SceUID moduleId = -1;

tai_hook_ref_t sceSysmoduleLoadModuleInternalWithArg_ref;
int sceSysmoduleLoadModuleInternalWithArg_patch(SceSysmoduleInternalModuleId id, SceSize args, void *argp, const SceSysmoduleOpt *option){

	int res, stat;

	if(id == SCE_SYSMODULE_INTERNAL_PAF){
		moduleId = sceKernelLoadStartModule("ux0:data/libpaf.suprx", args, argp, 0, NULL, &stat);

		res = (moduleId < 0) ? moduleId : 0;
		if(res == SCE_OK){
			*(option->result) = stat;
		}

	}else{
		res = TAI_CONTINUE(int, sceSysmoduleLoadModuleInternalWithArg_ref, id, args, argp, option);
	}

	return res;
}

tai_hook_ref_t sceSysmoduleIsLoadedInternal_ref;
int sceSysmoduleIsLoadedInternal_patch(SceSysmoduleInternalModuleId id){

	int res;

	if(id == SCE_SYSMODULE_INTERNAL_PAF){
		res = (moduleId < 0) ? SCE_SYSMODULE_ERROR_UNLOADED : 0;
	}else{
		res = TAI_CONTINUE(int, sceSysmoduleIsLoadedInternal_ref, id);
	}

	return res;
}

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp){

	HookImport(TAI_MAIN_MODULE, 0x03FCF19D, 0x09A4AF1D, sceSysmoduleIsLoadedInternal);
	HookImport(TAI_MAIN_MODULE, 0x03FCF19D, 0xC3C26339, sceSysmoduleLoadModuleInternalWithArg);

	return SCE_KERNEL_START_SUCCESS;
}
