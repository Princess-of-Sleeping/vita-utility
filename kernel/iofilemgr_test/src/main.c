
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

int write_file(const char *path, const void *data, SceSize size){

	if(data == NULL || size == 0)
		return -1;

	SceUID fd = ksceIoOpen(path, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0666);
	if (fd < 0)
		return fd;

	ksceIoWrite(fd, data, size);
	ksceIoClose(fd);

	return 0;
}

typedef struct SceVfsNode SceVfsNode;

typedef struct vfs_fd_lock {
   SceUID mutex_SceVfsFdLock;
   SceUID cond_SceVfsFdCond;
   uint32_t unk_8;
   uint32_t toggle; //inc before condition wait. dec after condition wait.

} vfs_fd_lock;

typedef struct SceVfsObjFileInfo {
	char *file;
	int file_len;
	int some_index;
	int unkC;
	int unk10;
	int unk14;
	int flags;
	SceMode mode;
} SceVfsObjFileInfo;

typedef struct vfs_node_70 { // size is 0x20
	struct vfs_node_70 *unk_0;
	struct vfs_node_70 *unk_4;
	struct vfs_node_70 *unk_8;
	struct vfs_node_70 *unk_C; //zero or pointer to next element
	SceVfsNode *unk_10;
	uint32_t unk_14;           //number
	SceSize file_name_len;
	char *file_name;
} vfs_node_70;

typedef struct SceVfsNode { //size is 0x100
	uint32_t unk_0;
	SceUID tid_4;                  // SceUID of thread in which node was created
	uint32_t some_counter_8;       // counter
	SceUID event_flag_SceVfsVnode; // 0xC - event flag SceVfsVnode

	uint32_t evid_bits;            // 0x10
	uint32_t unk_14;
	uint32_t unk_18;
	uint32_t unk_1C;

	uint8_t data1[0x20];

	void *ops;                     // maybe vop function table
	uint32_t unk_44;
	void *dev_info;                // allocated on heap with uid from uid field
                                       // this is device specific / node specific data
                                       // for partition node this will be vfs_device_info*
                                       // for pfs node it looks like to be child_node_info* 
                                       // which probably points to partition node

	void *mount; // size is 0x100

	struct SceVfsNode *prev_node;  // 0x50
	struct SceVfsNode *unk_54;     // copied from node base - singly linked list
	uint32_t some_counter_58;      // counter - probably counter of nodes
	void *unk_5C; // size is 0x58

	uint16_t *path; // u"/KD/BOOTIMAGE.SKPRX"
	uint32_t unk_64;
	uint32_t unk_68;
	SceUID pool_uid;               // 0x6C - SceIoVfsHeap or other pool

	vfs_node_70 *unk_70;
	uint32_t unk_74;               // = 0x8000
	uint32_t unk_78;               // some flag
	uint32_t unk_7C;

	uint32_t unk_80; // uid or func ptr
	uint32_t unk_84;
	uint32_t unk_88;
	uint32_t unk_8C;

	struct SceVfsNode *obj_internal_90; // this obj ptr?
	uint32_t maybe_internal_obj_count_94;
	struct SceVfsNode *child_node; // child node with deeper level of i/o implementation?
	uint32_t unk_9C;

	uint8_t data2[0x30];
   
	uint32_t unk_D0;               // devMajor.w.unk_4E

	uint8_t data3[0x2C];
} SceVfsNode;

typedef struct SceUIDVfsFileClassInternal { // size is 0x40
	int unk0;
	int unk4; // ex:0x1
	SceOff offset; // for example used in Read/Pread

	int flags; // 0x10 some flags
	SceUID pid1;
	SceVfsNode *node; // 0x18
	struct SceUIDVfsFileClassInternal *prev_obj;

	void *device_handle; // 0x20 - for Sdstor this will be sd_stor_device_handle*
	uint16_t unk24;
	uint8_t unk26;
	uint8_t unk27;
	vfs_fd_lock *fd_lock_ptr; // 0x28
	int devMinor; // 0x2C

	SceVfsObjFileInfo *pFileInfo; // NULL?
	int unk_0x34; // ex:0x1
	int unk_0x38; // ex:0x100
	SceUID pid2;
} SceUIDVfsFileClassInternal;

typedef struct SceUIDVfsFileClassObj { // size is 0x48
	int sce_rsvd[2];
	SceUIDVfsFileClassInternal info;
} SceUIDVfsFileClassObj;



void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp){

	SceUID SceIofilemgr_modid = ksceKernelSearchModuleByName("SceIofilemgr");

	void *iofilemgr_data = NULL;

	module_get_offset(0x10005, SceIofilemgr_modid, 1, 0, (uintptr_t *)&iofilemgr_data);

	SceClass *pSceUIDVfsFileClass = (SceClass *)(iofilemgr_data + 0x17C); // 0x8102117C - 0x81021000

	SceUID fd = ksceIoOpen("os0:kd/bootimage.skprx", SCE_O_RDONLY, 0);

	SceUIDVfsFileClassObj *obj = NULL;

	ksceKernelGetObjForUid(fd, pSceUIDVfsFileClass, (SceObjectBase **)&obj);
	ksceKernelUidRelease(fd);

	ksceDebugPrintf("fd   : 0x%X\n", fd);
	ksceDebugPrintf("obj  :0x%X\n", obj);
	ksceDebugPrintf("mount:0x%X\n", obj->info.node->mount);
	ksceDebugPrintf("%s\n", obj->info.node->unk_70->file_name);

	write_file("sd0:SceUIDVfsFileClass_obj.bin", obj->info.node->ops, 0x100);

	ksceIoClose(fd);

	return SCE_KERNEL_START_SUCCESS;
}
