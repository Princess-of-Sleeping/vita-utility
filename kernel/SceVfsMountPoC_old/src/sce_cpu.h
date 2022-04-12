/*
 * SceVfs mount PoC
 * Copyright (C) 2020 Princess of Sleeping
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef _PSP2_CPU_C_H_
#define _PSP2_CPU_C_H_

#include <psp2kern/types.h>

int init_l2_cache_reg(void);

int memcpy_rx(SceUID pid, void *dst, const void *src, SceSize len);

#endif // _PSP2_CPU_C_H_
