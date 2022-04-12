/*
 * SceVfs mount PoC
 * Copyright (C) 2020 Princess of Sleeping
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef _PSP2_KERNEL_VFS_H_
#define _PSP2_KERNEL_VFS_H_

#include <psp2kern/kernel/threadmgr.h>
#include <psp2kern/kernel/sysmem.h>
#include <psp2kern/io/dirent.h>

typedef struct SceVfsNode SceVfsNode;
typedef struct SceVfsFileObject SceVfsFileObject;
typedef struct SceVfsMountInfo SceVfsMountInfo;
typedef struct SceVfsMount2 SceVfsMount2;
typedef struct SceVfsAddParam SceVfsAddParam;

typedef struct SceVfsPath { //size is 0xC-bytes
	char *name;
	SceSize name_len;
	char *path;
} SceVfsPath;

typedef struct SceVfsopSetRootArgs { // size is 0xC-bytes
	SceVfsMountInfo *pMount;
	int data_0x04;
	SceVfsNode *pNode;
} SceVfsopSetRootArgs;

typedef struct SceVfsDevctl {
	void *unk_0x00;
	const char *dev;
	unsigned int cmd;
	void *indata;
	SceSize inlen;
	void *outdata;
	SceSize outlen;
} SceVfsDevctl;

typedef struct SceVfsTable { // size is 0x34-bytes
	const void *func00; // called by 0x810118cd/vfs_mount
	const void *func04; // called by 0x81011941/vfs_umount
	int (* vfs_set_root)(SceVfsopSetRootArgs *argp); // called by sceVfsMount->0x81011af1/vfs_set_root
	const void *func0C;
	const void *func10;
	const void *func14;
	const void *func18; // called by 0x81011b2d
	const void *func1C;
	const void *func20; // Called on ksceVfsAddVfs/vfs_init
	const void *func24; // Called on ksceVfsDeleteVfs
	const void *func28;
	int (* vfs_devctl)(SceVfsDevctl *args);
	const void *func30; // called by 0x81011bc5
} SceVfsTable;

/* vop args */

typedef struct SceVfsOpen { // size is 0xC-bytes
	SceVfsNode *node;
	SceVfsPath *file_info;
	int flags;
	SceVfsFileObject *pVfsFileObject;
} SceVfsOpen;

typedef struct SceVopCreateArgs { // size is 0x14-bytes
	SceVfsNode *pNode;
	SceVfsNode **ppNewNode;
	SceVfsPath *file_info;
	int flags;
	int mode;
} SceVopCreateArgs;

typedef struct SceVfsClose { // size is 0x8-bytes
	SceVfsNode *node;
	SceVfsFileObject *pVfsFileObject;
} SceVfsClose;

typedef struct SceVopPartPath { // size is 0x10-bytes
	const char *name;
	const char *msg;
	const char *path;
	const char *name2;
} SceVopPartPath;

typedef struct SceVfsPartInit { // size is 0x10-bytes
	SceVfsNode  *node;
	SceVfsNode **new_node; // result
	SceVopPartPath *path_info;
	SceUInt32 flags;
} SceVfsPartInit;

typedef struct SceVfsPartDeinit { // size is 0x10-bytes
	SceVfsNode *node;
	SceVfsNode *node2;
	SceVfsPath *file_info;
	SceUInt32 flags;
} SceVfsPartDeinit;

typedef struct SceVfsRead { // size is 0x10-bytes
	SceVfsNode *node;
	SceVfsFileObject *pVfsFileObject;
	void *data;
	SceSize nbyte;
} SceVfsRead;

typedef struct SceVfsWrite { // size is 0x10-bytes
	SceVfsNode *node;
	SceVfsFileObject *pVfsFileObject;
	const void *data;
	SceSize nbyte;
} SceVfsWrite;

typedef struct SceVfsDopen { // size is 0xC-bytes
	SceVfsNode *node;
	SceVfsPath *file_info;
	SceVfsFileObject *pVfsFileObject;
} SceVfsDopen;

typedef struct SceVfsDclose { // size is 0x8-bytes
	SceVfsNode *node;
	SceVfsFileObject *pVfsFileObject;
} SceVfsDclose;

typedef struct SceVfsDread { // size is 0xC-bytes
	SceVfsNode *node;
	SceVfsFileObject *pVfsFileObject;
	SceIoDirent *dir;
} SceVfsDread;

typedef struct SceVfsStat { // size is 0xC-bytes
	SceVfsNode *node;
	SceVfsPath *file_info;
	SceIoStat *stat;
} SceVfsStat;

typedef struct SceVfsStatByFd { // size is 0xC-bytes
	SceVfsNode *node;
	SceVfsFileObject *pVfsFileObject;
	SceIoStat *stat;
} SceVfsStatByFd;

typedef struct SceVopInactiveArgs { // size is 0x4-bytes
	SceVfsNode *node;
} SceVopInactiveArgs;

typedef struct SceVopLseekArgs { // size is 0x14-bytes
	SceVfsNode *node;
	SceVfsFileObject *pVfsFileObject;
	SceOff offset;
	int where;
} SceVopLseekArgs;

typedef struct SceVopIoctlArgs { // size is 0x1C-bytes
	SceVfsNode *node;
	SceVfsFileObject *pVfsFileObject;
    int cmd;
    const void *indata;
    SceSize inlen;
    void *outdata;
    SceSize outlen;
} SceVopIoctlArgs;

typedef struct SceVopMkdirArgs { // size is 0x10-bytes
	SceVfsNode  *node;
	SceVfsNode **pNewNode;
	SceVfsPath *file_info;
	int mode;
} SceVopMkdirArgs;

typedef struct SceVopRmdirArgs { // size is 0xC-bytes
	SceVfsNode *node;
	SceVfsNode *node2;
	SceVfsPath *file_info;
} SceVopRmdirArgs;

typedef struct SceVopChstatArgs { // size is 0x10-bytes
	SceVfsNode *node;
	SceVfsPath *file_info;
	SceIoStat *stat;
	int bit;
} SceVopChstatArgs;

typedef struct SceVopChstatByFdArgs { // size is 0x10-bytes
	SceVfsNode *pNode;
	SceVfsFileObject *pVfsFileObject;
	SceIoStat *stat;
	SceUInt32 bit;
} SceVopChstatByFdArgs;

typedef struct SceVopSyncArgs { // size is 0xC-bytes
	SceVfsNode *node;
	SceVfsFileObject *pVfsFileObject;
	int flags;
} SceVopSyncArgs;

typedef struct SceVopRenameArgs { // size is 0x18-bytes
	SceVfsNode *node;
	SceVfsNode *node2;
	SceVfsPath *file_info_old;
	SceVfsNode *node3;
	SceVfsNode **ppNewNode;
	SceVfsPath *file_info_new;
} SceVopRenameArgs;

typedef struct SceVopPreadArgs { // size is 0x18-bytes
	SceVfsNode *node;
	SceVfsFileObject *pVfsFileObject;
    void *data;
    SceSize nbyte;
    SceOff offset;
} SceVopPreadArgs;

typedef struct SceVopPwriteArgs { // size is 0x18-bytes
	SceVfsNode *node;
	SceVfsFileObject *pVfsFileObject;
    const void *data;
    SceSize nbyte;
    SceOff offset;
} SceVopPwriteArgs;

typedef struct SceVopTable { // size is 0x74-bytes
	int (* vop_open)(SceVfsOpen *argp);
	int (* vop_create)(SceVopCreateArgs *argp);
	int (* vop_close)(SceVfsClose *argp);
	int (* vop_lookup)(SceVfsPartInit *argp);             // SceIofilemgrForDriver_A5A6A55C/sceVfsNodeInitializePartition
	SceSSize (* vop_read)(SceVfsRead *argp);
	SceSSize (* vop_write)(SceVfsWrite *argp);
	SceOff (* vop_lseek)(SceVopLseekArgs *argp);
	int (* vop_ioctl)(SceVopIoctlArgs *argp);
	int (* vop_remove)(SceVfsPartDeinit *argp);           // SceIofilemgrForDriver_DC1E7EE4/sceVfsNodeDeinitializePartition?
	int (* vop_mkdir)(SceVopMkdirArgs *argp);
	int (* vop_rmdir)(SceVopRmdirArgs *argp);
	int (* vop_dopen)(SceVfsDopen *argp);
	int (* vop_dclose)(SceVfsDclose *argp);
	int (* vop_dread)(SceVfsDread *argp);
	int (* vop_get_stat)(SceVfsStat *argp);
	int (* vop_chstat)(SceVopChstatArgs *argp);
	int (* vop_rename)(SceVopRenameArgs *argp);
	const void *func44;                                   // sdstor op
	SceSSize (* vop_pread)(SceVopPreadArgs *argp);
	SceSSize (* vop_pwrite)(SceVopPwriteArgs *argp);
	int (* vop_inactive)(SceVopInactiveArgs *argp);
	const void *func54;                                   // SceIofilemgrForDriver_942AA61F/unknown
	const void *func58;                                   // SceIofilemgrForDriver_0D8A806E/unknown
	int (* vop_sync)(SceVopSyncArgs *argp);
	int (* vop_get_stat_by_fd)(SceVfsStatByFd *argp);     // SceIofilemgrForDriver_1DBCBB01/sceVopGetStatByFd?
	int (* vop_chstat_by_fd)(SceVopChstatByFdArgs *argp); // SceIofilemgrForDriver_082AFD7F/t_sceIoChstatByFdForDriver?
	const void *func68;                                   // SceIofilemgrForDriver_F53399BC/called by sceIoRemove
	const void *func6C;                                   // SceIofilemgrForDriver_0F7E1718/Called when sceVfsClose fails 4 times at the process exit or kill.
	const void *func70;                                   // SceIofilemgrForDriver_EEAE8B51/sdstor op
} SceVopTable;

typedef struct SceVfsNode SceVfsNode;

typedef struct SceVfsMountInfo { // size is 0x100-bytes
	SceKernelLwMutexWork lw_mtx;
	int data_0x20;
	int data_0x24;
	int data_0x28;
	int data_0x2C;
	int data_0x30;
	int data_0x34;
	int data_0x38;
	int data_0x3C;
	SceVfsNode *data_0x40;
	int data_0x44;
	int data_0x48;
	int data_0x4C; // some flags. copied from (SceVfsMountParam *(NULL))->data_0x08
	int data_0x50; // some flags. copied from ((SceVfsMountParam *(NULL))->data_0x0C & 0xFFFFF)
	SceVfsNode *pNodeTop;
	SceUInt32 nNodes;
	const SceVfsAddParam *pVfsAddParam;
	int data_0x60;
	int data_0x64;
	int data_0x68;
	int data_0x6C;
	int data_0x70;
	int data_0x74;
	struct SceVfsMountInfo *next; // maybe
	SceVfsMount2 *pVfsMount2;
	char device[0x40];
	int data_0xC0;
	void *data_0xC4;
	void *data_0xC8; // pointer to mutex_id
	void *prev;      // pointer to data_0xE0

	SceUID mutex_id;
	SceUID cond_id;
	int data_0xD8;
	int data_0xDC;

	int data_0xE0;
	int data_0xE4;
	void *data_0xE8;
	void *data_0xEC;

	int data_0xF0;
	int data_0xF4;
	int data_0xF8;
	int data_0xFC;
} SceVfsMountInfo;

typedef struct SceVfsNode { // size is 0x100-bytes
	uint32_t unk_0;
	SceUID tid_4;                  // SceUID of thread in which node was created
	uint32_t some_counter_8;       // counter
	SceUID event_flag_SceVfsVnode; // 0xC - event flag SceVfsVnode

	uint32_t evid_bits;            // 0x10
	uint32_t unk_14;
	uint32_t unk_18;
	uint32_t unk_1C;

	uint8_t data1[0x20];

	struct { // offset:0x40
		SceVopTable *vop_table;
		uint32_t unk_44;
		void *dev_info;                // allocated on heap with uid from uid field
                                       // this is device specific / node specific data
                                       // for partition node this will be vfs_device_info*
                                       // for pfs node it looks like to be child_node_info* 
                                       // which probably points to partition node

		SceVfsMountInfo *mount;        // 0x4C. copied from base node

		struct SceVfsNode *prev_node;  // 0x50

		struct SceVfsNode *unk_54;     // copied from node base - singly linked list
		uint32_t some_counter_58;      // counter - probably counter of nodes
		SceVfsTable *vfs_table;

		uint32_t unk_60;
		uint32_t unk_64;
		uint32_t unk_68;
		SceUID pool_uid;               // 0x6C - SceIoVfsHeap or other pool

		void *unk_70;
		uint32_t unk_74;               // = 0x8000
		uint32_t unk_78;               // some flag
		uint32_t unk_7C;

		SceOff st_size;
		SceUInt32 st_attr;
		uint32_t unk_8C;

		SceVfsFileObject *pVfsFileObject;
		uint32_t maybe_internal_obj_count_94;
		struct SceVfsNode *child_node; // child node with deeper level of i/o implementation?
		uint32_t unk_9C;

		uint8_t data2[0x30];

		uint32_t unk_D0;               // devMajor.w.unk_4E
		uint32_t unk_D4;
	} core;

	uint8_t data3[0x28];
} SceVfsNode;

typedef struct SceVfsFileObject { // size is 0x40-bytes
  SceBool is_dir_handler;
  int flags;                         // open flags
  SceOff offset;
  int flags2;                        // 0x10 some flags
  SceUID pid;
  SceVfsNode *node;                  // 0x18
  struct SceVfsFileObject *prev_obj;
  void *device_handle;               //0x20 - for Sdstor this will be sd_stor_device_handle*
  uint16_t unk24;
  uint8_t unk26;
  uint8_t unk27;
  void *fd_lock_ptr;                 // 0x28
  int devMinor;                      // 0x2C
  void *file_info; // size is 0x20-bytes. If have Dipsw 0xD2, only.
  char unk34[12];
} SceVfsFileObject;

typedef struct SceUIDVfsFileObject {
	void *userdata;
	SceClass *sce_class; 
	SceVfsFileObject vfs_object;
} SceUIDVfsFileObject;

typedef struct SceVfsAddParam { // size is 0x20-bytes
	const SceVfsTable *pVfsTable;
	const char *device;    // ex:"bsod_dummy_host_fs"
	int data_0x08;         // ex:0x11
	int is_mounted;
	int data_0x10;         // ex:0x10
	const SceVopTable *pVopTable;
	int data_0x18;
	struct SceVfsAddParam *prev;
} SceVfsAddParam;

typedef struct SceVfsMount2 { // size is 0x14
	const char *unit;
	const char *device1;
	const char *device2;
	const char *device3;
	int data_0x10;
} SceVfsMount2;

typedef struct SceVfsMountParam {   // size is 0x20
	const char *device;    // ex:"/host"
	int data_0x04;
	int data_0x08;         // ex:0x03000004

	/*
	 * flags ex:0x00008006
	 * mask 0x000000FF : device
	 *      1 : pfs
	 *      2 : unknown. another fs import?
	 *      3 : virtual device. like tty/md
	 *      4 : if data_0x04 != 0, success, but nop.
	 *      5 : ?
	 *      6 : host0:
	 * mask 0x0000FF00 : access?
	 *      0x1000 : Read only
	 */
	int data_0x0C;
	const char *data_0x10; // ex:"bsod_dummy_host_fs"
	int data_0x14;
	const SceVfsMount2 *pVfsMount2;
	int data_0x1C;
} SceVfsMountParam;

typedef struct SceVfsUmountParam {
	const char *device; // ex:"/host"
	int data_0x04;
} SceVfsUmountParam;

/* vfs function */
int ksceVfsAddVfs(SceVfsAddParam *pParam);
int ksceVfsDeleteVfs(const char *fs, SceVfsAddParam **pParam);
int ksceVfsMount(const SceVfsMountParam *pParam);
int ksceVfsUnmount(const SceVfsUmountParam *pParam);
int ksceVfsGetNewNode(SceVfsMountInfo *mount, const SceVopTable *pTable, int a3, SceVfsNode **ppNode);
int ksceVfsFreeVnode(SceVfsNode *pNode);
int ksceVfsNodeWaitEventFlag(SceVfsNode *pNode);
int ksceVfsNodeSetEventFlag(SceVfsNode *pNode);

/* vop function */
int ksceVopOpen(SceVfsNode *pNode, SceVfsPath *file_info, int flags, SceVfsFileObject *pVfsFileObject);
int ksceVopCreate(SceVfsNode *pNode, SceVfsNode **ppNewNode, SceVfsPath *file_info, int flags, int mode);
int ksceVopClose(SceVfsNode *pNode, SceVfsFileObject *pVfsFileObject);
int ksceVfsNodeInitializePartition(SceVfsNode *pNode, SceVfsNode **ppNewNode, SceVopPartPath *path_info, SceUInt32 flags); // ksceVopLookup
int ksceVopRead(SceVfsNode *pNode, SceVfsFileObject *pVfsFileObject, void *data, SceSize nbyte, SceSize *pResult);
int ksceVopWrite(SceVfsNode *pNode, SceVfsFileObject *pVfsFileObject, const void *data, SceSize nbyte, SceSize *pResult);
SceOff ksceVopLseek(SceVfsNode *pNode, SceVfsFileObject *pVfsFileObject, SceOff offset, int where);
int ksceVopIoctl(SceVfsNode *pNode, SceVfsFileObject *pVfsObject, int cmd, const void *indata, SceSize inlen, void *outdata, SceSize outlen);
int ksceVopRemove(SceVfsNode *pNode, SceVfsNode *pNode2, SceVfsPath *file_info, SceUInt32 flags); // SceIofilemgrForDriver_DC1E7EE4
int ksceVopMkdir(SceVfsNode *pNode,SceVfsNode **ppNewNode, const char *dir, int mode);
int ksceVopRmdir(SceVfsNode *pNode, SceVfsNode *pNode2, SceVfsPath *file_info);
int ksceVopDopen(SceVfsNode *pNode, SceVfsPath *file_info, SceVfsFileObject *pVfsFileObject);
int ksceVopDclose(SceVfsNode *pNode, SceVfsFileObject *pVfsFileObject);
int ksceVopDread(SceVfsNode *pNode, SceVfsFileObject *pVfsFileObject, SceIoDirent *dir);
int ksceVopGetstat(SceVfsNode *pNode, SceVfsPath *file_info, SceIoStat *stat);
int ksceVopChstat(SceVfsNode *pNode, SceVfsPath *file_info, SceIoStat *stat, int bit);
int ksceVopRename(SceVfsNode *pNode, SceVfsNode *pNode2, SceVfsPath *file_info_old, SceVfsNode *node3, SceVfsNode **ppNewNode, SceVfsPath *file_info_new);
int ksceVopPread(SceVfsNode *pNode, SceVfsFileObject *pVfsFileObject, void *data, SceSize nbyte, SceOff offset, SceSize *pResult);
int ksceVopPwrite(SceVfsNode *pNode, SceVfsFileObject *pVfsFileObject, const void *data, SceSize nbyte, SceOff offset, SceSize *pResult);
int ksceVopInactive(SceVfsNode *pNode);
int ksceVopSync(SceVfsNode *pNode, SceVfsFileObject *pVfsFileObject, int flags);
int ksceVopGetStateByFd(SceVfsNode *pNode, SceVfsFileObject *pVfsFileObject, SceIoStat *stat);
int ksceVopChstatByFd(SceVfsNode *pNode, SceVfsFileObject *pVfsFileObject, SceIoStat *stat, int bit);

#endif // _PSP2_KERNEL_VFS_H_
