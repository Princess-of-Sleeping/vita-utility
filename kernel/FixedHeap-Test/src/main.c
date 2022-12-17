
#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/sysmem.h>
#include <psp2kern/kernel/debug.h>


int module_get_export_func(SceUID pid, const char *modname, SceNID libnid, SceNID funcnid, uintptr_t *func);


typedef struct SceKernelFixedHeapHook { // size is 0xC-bytes
	int unk_0x00;
	void *(* funcAlloc)(int a1, SceSize length);
	void (* funcFree)(int a1, ScePVoid ptr);
} SceKernelFixedHeapHook;


int (* sceKernelCreateObjectHeap)(SceUID pid, SceBool a2, SceKernelFixedHeapHook *pHeapHook, void **result);

void *(* sceFixedHeapAllocObject)(void *pFixedHeap);
int (* sceFixedHeapFreeObject)(void *pFixedHeap, void *pObject);

SceKernelFixedHeapHook hook;



void *my_funcAlloc(int a1, SceSize length){
	return ksceKernelAlloc(length);
}

void my_funcFree(int a1, ScePVoid ptr){
	ksceKernelFree(ptr);
}

/*
 * FixedHeap:
 *   have cpuCache (?).
 *   have page. 0x1000-bytes.
 *   max nItem is (0x1000 / itemSize) on 1-page.
 */

void _start() __attribute__ ((weak, alias ("module_start")));
int module_start(SceSize args, void *argp){

	ksceDebugPrintf("AAA\n");


	module_get_export_func(0x10005, "SceSysmem", 0x63A519E5, 0x36830F46, (uintptr_t *)&sceKernelCreateObjectHeap);
	module_get_export_func(0x10005, "SceSysmem", 0x63A519E5, 0xC8672A3D, (uintptr_t *)&sceFixedHeapAllocObject);
	module_get_export_func(0x10005, "SceSysmem", 0x63A519E5, 0x571660AA, (uintptr_t *)&sceFixedHeapFreeObject);

	int res;
	void *result;

	hook.unk_0x00 = 0;
	hook.funcAlloc = my_funcAlloc;
	hook.funcFree = my_funcFree;

	res = sceKernelCreateObjectHeap(0x10005, SCE_TRUE, &hook, &result);

	ksceDebugPrintf("sceKernelCreateObjectHeap: 0x%X\n", res);
	if(res >= 0){
		ksceDebugPrintf("result: 0x%X\n", result);

		void *pFixedHeap = *(void **)(result + 0x18); // default 0x40-bytes

		void *alloc_res[0x40];

		for(int i=0;i<0x40;i++){
			alloc_res[i] = sceFixedHeapAllocObject(pFixedHeap);
			ksceDebugPrintf("alloc_res: %d-0x%X\n", i, alloc_res[i]);
		}

		sceFixedHeapFreeObject(pFixedHeap, alloc_res[2]);

		alloc_res[2] = sceFixedHeapAllocObject(pFixedHeap);
		ksceDebugPrintf("alloc_res: 0x%X\n", alloc_res[2]);

		alloc_res[2] = sceFixedHeapAllocObject(pFixedHeap);
		ksceDebugPrintf("alloc_res: 0x%X\n", alloc_res[2]);
	}

	return SCE_KERNEL_START_SUCCESS;
}
