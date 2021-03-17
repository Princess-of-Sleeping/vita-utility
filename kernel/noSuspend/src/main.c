/*
 * PS Vita noSuspend
 * Copyright (C) 2021 Princess of Sleeping
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include <psp2kern/kernel/modulemgr.h>

void scePowerSuspendCallback(int a1, int a2, int a3){}

typedef void (* ScePowerIdleCallback)(int a1, int a2, int a3);

int kscePowerSetIdleCallback(int a1, int inhibit_reset, SceUInt64 time, ScePowerIdleCallback cb, int a6);


void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp){

	kscePowerSetIdleCallback(0, 0, 0LL, NULL, 0); // clear
	kscePowerSetIdleCallback(0, 0, 0LL, scePowerSuspendCallback, 0);

	return SCE_KERNEL_START_SUCCESS;
}
