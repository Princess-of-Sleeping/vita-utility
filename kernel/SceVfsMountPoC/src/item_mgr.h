/*
 * SceVfs mount PoC
 * Copyright (C) 2020 Princess of Sleeping
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef _PSP2_ITEM_MGR_H_
#define _PSP2_ITEM_MGR_H_

#include <psp2kern/types.h>
#include "fs_mgr.h"
#include "vfs.h"

int init_itemmgr(void);

int create_file_item(void *node, DataPackForSystem_t **ppDataPackForSystem);

void *getFileEntryByNode(const SceVfsNode *node);

#endif // _PSP2_ITEM_MGR_H_
