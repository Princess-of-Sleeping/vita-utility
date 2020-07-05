
#ifndef _PSP2_KERNEL_VFS_H_
#define _PSP2_KERNEL_VFS_H_

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

	void *ops;                     // 0x40
	uint32_t unk_44;
	void *dev_info;                // allocated on heap with uid from uid field
                                       // this is device specific / node specific data
                                       // for partition node this will be vfs_device_info*
                                       // for pfs node it looks like to be child_node_info* 
                                       // which probably points to partition node

	void *mount;                   // 0x4C

	struct SceVfsNode *prev_node;  // 0x50

	struct SceVfsNode *unk_54;     // copied from node base - singly linked list
	uint32_t some_counter_58;      // counter - probably counter of nodes
	void *unk_5C;

	uint32_t unk_60;
	uint32_t unk_64;
	uint32_t unk_68;
	SceUID pool_uid;               // 0x6C - SceIoVfsHeap or other pool

	void *unk_70;
	uint32_t unk_74;               // = 0x8000
	uint32_t unk_78;               // some flag
	uint32_t unk_7C;

	uint32_t unk_80;
	uint32_t unk_84;
	uint32_t unk_88;
	uint32_t unk_8C;

	void *obj_internal_90;
	uint32_t maybe_internal_obj_count_94;
	struct SceVfsNode *child_node; // child node with deeper level of i/o implementation?
	uint32_t unk_9C;

	uint8_t data2[0x30];
   
	uint32_t unk_D0;               // devMajor.w.unk_4E

	uint8_t data3[0x2C];
} SceVfsNode;

typedef struct SceVfsNodePartInitArgs {
	SceVfsNode  *node; 
	SceVfsNode **new_node; // result
	void *opt;
	uint32_t flags; 
} SceVfsNodePartInitArgs;

int ksceVfsGetNewNode(void *mount, int a2, int a3, SceVfsNode **ppNode);
int ksceVfsNodeWaitEventFlag(SceVfsNode *pNode);

#endif // _PSP2_KERNEL_VFS_H_
