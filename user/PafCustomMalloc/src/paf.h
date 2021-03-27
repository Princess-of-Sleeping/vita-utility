
#ifndef _MY_PSP2_PAF_H_
#define _MY_PSP2_PAF_H_

#include <psp2/types.h>
#include <psp2/kernel/threadmgr.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ScePafInit { // size is 0x18
	SceSize global_heap_size;
	int a2;
	int a3;
	int use_gxm;
	int heap_opt_param1;
	int heap_opt_param2;
} ScePafInit;

typedef struct ScePafHeapInfo { // size is 0x60
	void *a1;                    // some paf ptr
	void *a2;                    // membase
	void *a3;                    // membase top
	SceSize size;
	char name[0x20];
	char data_0x30[4];
	int data_0x34;
	SceKernelLwMutexWork lw_mtx;
	SceUID memblk_id;
	int data_0x5C;               // ex:1
} ScePafHeapInfo;

typedef struct ScePafHeapOpt { // size is 0x14
	int a1;
	int a2;
	char a3[4];
	int a4;
	int a5;
} ScePafHeapOpt;

ScePafHeapInfo *scePafHeapInit(ScePafHeapInfo *pInfo, void *membase, SceSize size, const char *name, ScePafHeapOpt *pOpt);
ScePafHeapInfo *scePafHeapFini(ScePafHeapInfo *pInfo);

void *scePafMallocWithInfo(ScePafHeapInfo *pInfo, SceSize len);

#ifdef __cplusplus
}
#endif

#endif /* _MY_PSP2_PAF_H_ */
