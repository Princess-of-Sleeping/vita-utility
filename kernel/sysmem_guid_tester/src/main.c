/*
 * PSVita GUID Tester
 * Copyright (C) 2021, Princess of Sleeping
 */

#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/sysmem.h>
#include <taihen.h>

#define GetExport(modname, libnid, funcnid, func) module_get_export_func(0x10005, modname, libnid, funcnid, (uintptr_t *)func)

int module_get_export_func(SceUID pid, const char *modname, uint32_t libnid, uint32_t funcnid, uintptr_t *func);
int module_get_offset(SceUID pid, SceUID modid, int segidx, size_t offset, uintptr_t *addr);

const char *(* sceKernelGetNameForUid2)(SceUID uid);
SceClass *(* sceGUIDtoClass)(SceUID uid);

/*
 * Openable only if the reference count of the target guid is zero
 */
#define SCE_GUID_REFER_ATTR_NO_REFERRED_OPENABLE (1 << 0)

int (* sceGUIDReferObjectClassCheckCommon)(SceUID guid, int mode, SceClass *pClass, SceUInt32 level, int attr, SceObjectBase **ppObject);

typedef struct SceKernelUIDHeapClass { // size is 0x34-bytes
	SceClass sceUIDHeapClass;
	int data_0x2C;
	int data_0x30;
} SceKernelUIDHeapClass;

typedef struct SceKernelUIDFixedHeapClass { // size is 0x3C-bytes
	SceClass sceUIDFixedHeapClass;
	int data_0x2C;
	int data_0x30;
	int data_0x34;
	int data_0x38;
} SceKernelUIDFixedHeapClass;

// ModulePrivate3
typedef struct SceKernelSystemMemory { // size is 0xDC-bytes
	SceSize size;
	int data_0x04;
	void *data_0x08; // size is 0xB0-bytes. Related to SceUIDEntryHeapClass
	void *data_0x0C; // size is 0x8-bytes.

	void *data_0x10; // size is 0xAC-bytes. ScePhyMemPartKD
	void *data_0x14; // size is 0xAC-bytes. ScePhyMemPartTool
	int data_0x18;
	int data_0x1C; // copied from KernelBootArgs

	int data_0x20; // copied from KernelBootArgs
	int data_0x24; // copied from KernelBootArgs
	int data_0x28;
	int data_0x2C;

	int data_0x30;
	int data_0x34; // copied from KernelBootArgs
	void *data_0x38; // size is 0xAC-bytes. Related to SceKernelNameHeapHash
	int data_0x3C;

	void *data_0x40; // size is 0x80-bytes. Related to partition
	void *pKernelHeapObject;
	void *data_0x48; // size is 0xC4-bytes. like malloc context
	SceSize number_of_memory_base_list;

	struct {
		uintptr_t base;
		SceSize length;
	} address_base[4];

	// some TTBR stuff
	int data_0x70; // ex:0
	int data_0x74; // copied from KernelBootArgs
	int data_0x78; // copied from KernelBootArgs
	int data_0x7C; // copied from KernelBootArgs

	int data_0x80; // copied from KernelBootArgs
	// some TTBR stuff

	int data_0x84;
	int data_0x88;
	int data_0x8C;

	int data_0x90;
	int data_0x94;
	SceClass *pUIDClass;
	SceClass *pUIDDLinkClass;

	// offset:0xA0
	SceKernelUIDHeapClass      *pKernelUIDHeapClass;
	SceKernelUIDFixedHeapClass *pKernelUIDFixedHeapClass;
	void *data_0xA8; // size is 0x3C-bytes
	int data_0xAC;

	SceClass *pUIDMemBlockClass;
	SceClass *pUIDTinyPartitionClass;
	SceClass *pUIDPartitionClass;
	void *data_0xBC; // size is 0x34-bytes. Related to SceUIDKernelHeapClass

	void *data_0xC0; // size is 0xC-bytes
	int data_0xC4; // size is 0x18-bytes. Related to partition
	int data_0xC8; // size is 0x28-bytes. Related to partition
	SceUID kernelHeapUncachedId;

	void *data_0xD0; // SceAS object pointer
	void *data_0xD4; // size is 0xC-bytes. Related to SceUIDAddressSpaceClass
	int data_0xD8;
} SceKernelSystemMemory;

typedef struct SceGUIDEntry { // size is 0x20-bytes
	SceObjectBase *pObject;
	SceUID pid;
	SceUInt32 attr;
	SceUInt8 unused;
	SceUInt8 uid_register; //<! The max value is 0xF0
	SceUInt16 level;       //<! Aka refer count.
	SceUID guid;
	int unk_0x14;
	void *unk_0x18;
	char *name;
} SceGUIDEntry;

typedef struct SceUIDEntryHeapObject { // size is 0xB0-bytes
	void *pUserdata;
	SceClass *pClass;
	int data_0x08;
	int data_0x0C;
	int data_0x10;
	int data_0x14;
	int data_0x18;
	int data_0x1C;
	short entrySize;
	short data_0x22;
	short data_0x24;
	short data_0x26;
	int data_0x28;
	SceUInt32 magic;

	int data_0x30;
	int data_0x34; // nData_0x38
	SceGUIDEntry **data_0x38;
	SceGUIDEntry *data_0x3C;

	int data_0x40;
	short data_0x44;
	short data_0x46;
	int data_0x48;
	int data_0x4C;

	int data_0x50[24];
} SceUIDEntryHeapObject;

SceUInt32 (* sceUIDCoreSuspendCpuIntr)(void *pUIDCoreContext, const char *function, int line);
void (* sceUIDCoreResumeCpuIntr)(void *pUIDCoreContext, SceUInt32 prev_state);

int (* sceGUIDGetEntryWithAttr)(void *pUIDCoreContext, SceUID guid, int attr, SceGUIDEntry **ppEntry);

SceKernelSystemMemory **ppModulePrivate3 = NULL;

__attribute__((noinline, optimize("O2")))
void hex_dump(const void *addr, int len){

	if(addr == NULL)
		return;

	if(len == 0)
		return;

	while(len >= 0x10){
		ksceDebugPrintf(
			"%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n",
			((char *)addr)[0x0], ((char *)addr)[0x1], ((char *)addr)[0x2], ((char *)addr)[0x3],
			((char *)addr)[0x4], ((char *)addr)[0x5], ((char *)addr)[0x6], ((char *)addr)[0x7],
			((char *)addr)[0x8], ((char *)addr)[0x9], ((char *)addr)[0xA], ((char *)addr)[0xB],
			((char *)addr)[0xC], ((char *)addr)[0xD], ((char *)addr)[0xE], ((char *)addr)[0xF]
		);
		addr += 0x10;
		len -= 0x10;
	}

	if(len != 0){
		while(len >= 1){
			ksceDebugPrintf("%02X ", ((char *)addr)[0x0]);
			addr += 1;
			len -= 1;
		}

		ksceDebugPrintf("\n");
	}
}

int uid_entry_test(SceKernelSystemMemory *pModulePrivate3){

	SceUIDEntryHeapObject *pUIDEntryHeapObject = pModulePrivate3->data_0x08;

	for(int i=0;i<pUIDEntryHeapObject->data_0x34;i++){

		int uVar1;
		SceGUIDEntry *pSVar5 = pUIDEntryHeapObject->data_0x38[i];

		if (pUIDEntryHeapObject->data_0x3C == pSVar5) {
			uVar1 = pUIDEntryHeapObject->data_0x44;
		}else{
			uVar1 = pUIDEntryHeapObject->data_0x46;
		}

		for(int n=0;n<uVar1;n++){
			if((0x300000 & pSVar5[n].attr) != 0){

				if(NULL != pSVar5[n].pObject){
					ksceDebugPrintf("UID: 0x%08X, attr: 0x%08X, name: %s\n", pSVar5[n].guid, pSVar5[n].attr, pSVar5[n].name);
				}
			}
		}
	}

	return 0;
}

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp){

	int res;
	SceUID module_id, guid;
	void *pObject = NULL;

	module_id = ksceKernelSearchModuleByName("SceSysmem");

	module_get_offset(0x10005, module_id, 0, 0x3F18 | 1, (uintptr_t *)&sceGUIDReferObjectClassCheckCommon);
	module_get_offset(0x10005, module_id, 0, 0x11390 | 1, (uintptr_t *)&sceUIDCoreSuspendCpuIntr);
	module_get_offset(0x10005, module_id, 0, 0x1139C | 1, (uintptr_t *)&sceUIDCoreResumeCpuIntr);
	module_get_offset(0x10005, module_id, 0, 0x3E3C | 1, (uintptr_t *)&sceGUIDGetEntryWithAttr);
	module_get_offset(0x10005, module_id, 1, 0x144, (uintptr_t *)&ppModulePrivate3);

	GetExport("SceSysmem", 0x6F25E18A, 0xE655852F, &sceKernelGetNameForUid2);
	GetExport("SceSysmem", 0x63A519E5, 0x66636970, &sceGUIDtoClass);

	ksceDebugPrintf("%p\n", ppModulePrivate3);  // Should be in SceSysmem data segment
	ksceDebugPrintf("%p\n", *ppModulePrivate3);

	if(1){
		uid_entry_test(*ppModulePrivate3);
		return SCE_KERNEL_START_NO_RESIDENT;
	}

	guid = 0x10001;

	if(1){
		SceGUIDEntry *pEntry;
		SceUInt32 prev = sceUIDCoreSuspendCpuIntr((*ppModulePrivate3)->data_0x08, __FUNCTION__, __LINE__);

		res = sceGUIDGetEntryWithAttr((*ppModulePrivate3)->data_0x08, guid, 0, &pEntry);
		if(res >= 0){
			ksceDebugPrintf("Internal pEntry = %p\n", pEntry);
			ksceDebugPrintf("pObject = %p\n", pEntry->pObject);
			ksceDebugPrintf("pid     = 0x%08X\n", pEntry->pid);
			ksceDebugPrintf("attr    = 0x%08X\n", pEntry->attr);
			ksceDebugPrintf("level   = 0x%04X\n", pEntry->level);
			ksceDebugPrintf("guid    = 0x%08X\n", pEntry->guid);
		}

		sceUIDCoreResumeCpuIntr((*ppModulePrivate3)->data_0x08, prev);
	}

	if(guid >= 0){
		SceClass *cls = sceGUIDtoClass(guid);
		ksceDebugPrintf("0x%X : %-31s %-31s ;\n", guid, cls->name, sceKernelGetNameForUid2(guid));

		res = ksceKernelGetObjForUid(guid, cls, (SceObjectBase **)&pObject);
		if(res >= 0){
			ksceKernelUidRelease(guid);

			ksceDebugPrintf("pObject=%p\n", pObject);

			sceGUIDReferObjectClassCheckCommon(guid, 1, sceGUIDtoClass(guid), 7, 0, (SceObjectBase **)&pObject);
			ksceKernelUidRelease(guid);

			ksceDebugPrintf("pObject=%p\n", pObject);
		}
	}

	return SCE_KERNEL_START_NO_RESIDENT;
}
