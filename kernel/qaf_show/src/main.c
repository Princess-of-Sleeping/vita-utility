/*
 * SceQaf show
 * Copyright (C) 2021 Princess of Sleeping
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/debug.h>

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp){

	ksceDebugPrintf("sceSblQafMgrIsAllowControlIduAutoUpdate   :%d\n", ksceSblQafMgrIsAllowControlIduAutoUpdate());
	ksceDebugPrintf("sceSblQafMgrIsAllowDecryptedBootConfigLoad:%d\n", ksceSblQafMgrIsAllowDecryptedBootConfigLoad());
	ksceDebugPrintf("sceSblQafMgrIsAllowDtcpIpReset            :%d\n", ksceSblQafMgrIsAllowDtcpIpReset());
	ksceDebugPrintf("sceSblQafMgrIsAllowHost0Access            :%d\n", ksceSblQafMgrIsAllowHost0Access());
	ksceDebugPrintf("sceSblQafMgrIsAllowKeepCoreFile           :%d\n", ksceSblQafMgrIsAllowKeepCoreFile());
	ksceDebugPrintf("sceSblQafMgrIsAllowLoadMagicGate          :%d\n", ksceSblQafMgrIsAllowLoadMagicGate());
	ksceDebugPrintf("sceSblQafMgrIsAllowMarlinTest             :%d\n", ksceSblQafMgrIsAllowMarlinTest());
	ksceDebugPrintf("sceSblQafMgrIsAllowNearTest               :%d\n", ksceSblQafMgrIsAllowNearTest());
	ksceDebugPrintf("sceSblQafMgrIsAllowPSPEmuShowQAInfo       :%d\n", ksceSblQafMgrIsAllowPSPEmuShowQAInfo());
	ksceDebugPrintf("sceSblQafMgrIsAllowRemotePlayDebug        :%d\n", ksceSblQafMgrIsAllowRemotePlayDebug());
	ksceDebugPrintf("sceSblQafMgrIsAllowSystemAppDebug         :%d\n", ksceSblQafMgrIsAllowSystemAppDebug());

	return SCE_KERNEL_START_NO_RESIDENT;
}
