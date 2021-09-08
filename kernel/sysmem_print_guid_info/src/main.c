
#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/threadmgr.h>
#include <psp2kern/kernel/sysmem.h>
#include <psp2kern/kernel/sysclib.h>
#include <psp2kern/io/fcntl.h>
#include <taihen.h>

#define GetExport(modname, libnid, funcnid, func) module_get_export_func(0x10005, modname, libnid, funcnid, (uintptr_t *)func)

int module_get_export_func(SceUID pid, const char *modname, uint32_t libnid, uint32_t funcnid, uintptr_t *func);
int module_get_offset(SceUID pid, SceUID modid, int segidx, size_t offset, uintptr_t *addr);

#define HookExport(module_name, library_nid, func_nid, func_name) \
	taiHookFunctionExportForKernel(0x10005, &func_name ## _ref, module_name, library_nid, func_nid, func_name ## _patch)
#define HookImport(module_name, library_nid, func_nid, func_name) \
	taiHookFunctionImportForKernel(0x10005, &func_name ## _ref, module_name, library_nid, func_nid, func_name ## _patch)
#define HookOffset(modid, offset, thumb, func_name) \
	taiHookFunctionOffsetForKernel(0x10005, &func_name ## _ref, modid, 0, offset, thumb, func_name ## _patch)

#define HookRelease(hook_uid, hook_func_name)({ \
	(hook_uid > 0) ? taiHookReleaseForKernel(hook_uid, hook_func_name ## _ref) : -1; \
})

const char *(* sceKernelGetNameForUid2)(SceUID uid);
SceClass *(* sceGUIDtoClass)(SceUID uid);

int print_guid_info(SceUID uid){

	SceClass *cls = sceGUIDtoClass(uid);

	if(cls == NULL){
		ksceDebugPrintf("0x%X : %s\n", uid, "failed");
	}else{
		ksceDebugPrintf("0x%X : %-31s %-31s\n", uid, cls->name, sceKernelGetNameForUid2(uid));
	}

	return 0;
}

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp){

	GetExport("SceSysmem", 0x6F25E18A, 0xE655852F, &sceKernelGetNameForUid2);
	GetExport("SceSysmem", 0x63A519E5, 0x66636970, &sceGUIDtoClass);

	for(int i=0;i<0x400;i++)
		print_guid_info(0x10005 + (i << 1));

	return SCE_KERNEL_START_SUCCESS;
}
