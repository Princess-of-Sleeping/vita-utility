/*
 * PSVita Kernel Module Loader
 * Copyright (C) 2021, Princess of Sleeping
 */

#include <psp2/kernel/modulemgr.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/clib.h>
#include <psp2/io/fcntl.h>
#include <psp2/appmgr.h>
#include <psp2/paf.h>
#include <psp2/sysmodule.h>
#include <taihen.h>

const char    sceUserMainThreadName[]          = "kernel_module_loader";
const int     sceUserMainThreadPriority        = 0x40;
const int     sceUserMainThreadCpuAffinityMask = 1 << 0;
const SceSize sceUserMainThreadStackSize       = 0x4000;

const int sceKernelPreloadModuleInhibit = SCE_KERNEL_PRELOAD_INHIBIT_LIBC
					| SCE_KERNEL_PRELOAD_INHIBIT_LIBDBG
					| SCE_KERNEL_PRELOAD_INHIBIT_APPUTIL
					| SCE_KERNEL_PRELOAD_INHIBIT_LIBSCEFT2
					| SCE_KERNEL_PRELOAD_INHIBIT_LIBPERF;

typedef struct ScePafInit { // size is 0x18
	SceSize global_heap_size;
	int a2;
	int a3;
	int use_gxm;
	int heap_opt_param1;
	int heap_opt_param2;
} ScePafInit;

typedef struct ModuleLoadParam {
	struct ModuleLoadParam *next;
	char *path;
	SceSize path_length;
	SceUID modid;
} ModuleLoadParam;

int module_num = 0;
ModuleLoadParam *pModuleLoadParamTree = NULL;

int load_module_add_path(const char *path){

	if(path == NULL){
		return -3;
	}

	SceUID fd = sceIoOpen(path, SCE_O_RDONLY, 0);
	if(fd < 0){
		return -4;
	}

	sceIoClose(fd);

	ModuleLoadParam *pModuleLoadParam;

	pModuleLoadParam = sce_paf_private_malloc(sizeof(*pModuleLoadParam));
	if(pModuleLoadParam == NULL){
		return -1;
	}

	size_t path_length = sce_paf_private_strlen(path);
	if(path_length == 0){
		return -2;
	}

	char *tmp_path;

	tmp_path = sce_paf_private_malloc(path_length + 1);
	if(tmp_path == NULL){
		sce_paf_private_free(pModuleLoadParam);
		return -1;
	}

	sce_paf_private_memcpy(tmp_path, path, path_length + 1);

	pModuleLoadParam->next        = pModuleLoadParamTree;
	pModuleLoadParam->path        = tmp_path;
	pModuleLoadParam->path_length = path_length + 1;
	pModuleLoadParam->modid       = -1;

	pModuleLoadParamTree = pModuleLoadParam;
	module_num = module_num + 1;

	return 0;
}

ModuleLoadParam *saerch_module_entry(const char *path){

	ModuleLoadParam *pModuleLoadParam = pModuleLoadParamTree;

	while(pModuleLoadParam != NULL){
		if(sce_paf_private_strcmp(path, pModuleLoadParam->path) == 0)
			return pModuleLoadParam;

		pModuleLoadParam = pModuleLoadParam->next;
	}

	return NULL;
}

int tty_in_thread(SceSize args, void *argp){

	int res;
	char tty_buf[0x80];

	SceUID stdin = sceKernelGetStdin();

	sceClibPrintf("commands\n");
	sceClibPrintf("\tload module_path\n");
	sceClibPrintf("\texit\n");

	while(1){
		sceClibMemset(tty_buf, 0, sizeof(tty_buf));
		res = sceIoRead(stdin, tty_buf, sizeof(tty_buf) - 1);
		if(res < 0){
			continue;
		}

		tty_buf[res - 1] = 0;

		if(sceClibStrncmp(tty_buf, "load ", 5) == 0){

			ModuleLoadParam *param = saerch_module_entry(&tty_buf[5]);
			if(param == NULL){
				sceClibPrintf("not found module\n");
				continue;
			}
			param->modid = taiLoadStartKernelModule(param->path, 0, NULL, 0);
		}else if(sceClibStrncmp(tty_buf, "exit", 4) == 0){
			break;
		}
	}

	return 0;
}

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp){

	int load_res;
	ScePafInit init_param;
	SceSysmoduleOpt sysmodule_opt;

	init_param.global_heap_size = 0x800000;
	init_param.a2               = 0xFFFFFFFF;
	init_param.a3               = 0xFFFFFFFF;
	init_param.use_gxm          = 0;
	init_param.heap_opt_param1  = 1;
	init_param.heap_opt_param2  = 1;

	load_res = 0xFFFFFFFF;
	sysmodule_opt.flags  = 0x10; // with arg
	sysmodule_opt.result = &load_res;

	sceSysmoduleLoadModuleInternalWithArg(SCE_SYSMODULE_INTERNAL_PAF, sizeof(init_param), &init_param, &sysmodule_opt);

	load_module_add_path("host0:module.skprx");
	load_module_add_path("host0:baremetal-loader.skprx");
	load_module_add_path("host0:kbl-loader.skprx");
	load_module_add_path("ux0:module.skprx");
	load_module_add_path("ux0:kbl-loader.skprx");

	int count = 1;
	ModuleLoadParam *pModuleLoadParam = pModuleLoadParamTree;

	while(pModuleLoadParam != NULL){
		sceClibPrintf("%-4d %s\n", count, pModuleLoadParam->path);

		count++;
		pModuleLoadParam = pModuleLoadParam->next;
	}

	pModuleLoadParam = pModuleLoadParamTree;

	if(pModuleLoadParam == NULL){
		sceClibPrintf("no modules\n");
		goto exit_app;
	}

	if(module_num == 1){
load_first_module:
		pModuleLoadParam->modid = taiLoadStartKernelModule(pModuleLoadParam->path, 0, NULL, 0);
	}else{
		int search_unk[2];
		SceUID search_modid;
		search_modid = _vshKernelSearchModuleByName("SceDeci4pSTtyp", search_unk);
		if(search_modid < 0){
			sceClibPrintf("No devkit. load to first module.");
			sceClibPrintf("-> %s\n", pModuleLoadParam->path);
			goto load_first_module;
		}

		SceUID thid = sceKernelCreateThread("tty_in_thread", tty_in_thread, 0x10000100, 0x4000, 0, 0, NULL);
		sceKernelStartThread(thid, 0, NULL);
		int stat;
		sceKernelWaitThreadEnd(thid, &stat, NULL);
		sceKernelDeleteThread(thid);
	}

exit_app:
	sceAppMgrDestroyAppByAppId(~2);

	return SCE_KERNEL_START_SUCCESS;
}
