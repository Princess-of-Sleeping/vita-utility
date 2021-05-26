/*
 * PSVita act verifier
 * Copyright (C) 2021, Princess of Sleeping
 */

#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/threadmgr.h>
#include <psp2kern/kernel/sysclib.h>
#include <psp2kern/kernel/utils.h>
#include <psp2kern/kernel/ssmgr.h>
#include <psp2kern/kernel/debug.h>
#include <psp2kern/io/fcntl.h>
#include "aes256cmac.h"

int module_get_offset(SceUID pid, SceUID modid, int segidx, uint32_t offset, uintptr_t *dst);

int ksceSblNvsReadData(int offset, void *buffer, SceSize size);

typedef struct SceNpDrmRsaKey {
	const void *n;
	const void *k; // e/d
} SceNpDrmRsaKey;

typedef struct SceKitActivationDataToken { // size is 0x40
	char magic[4]; // "act\n"
	uint32_t issue_no;
	uint32_t format_version;
	uint32_t start_date;
	uint32_t end_date;
	char open_psid[0x10];
	char padding[0xC];
	char cmac[0x10];
} SceKitActivationDataToken;

typedef struct SceKitActivationData { // size is 0x80
	SceKitActivationDataToken plain;
	SceKitActivationDataToken enced;
} SceKitActivationData;

typedef struct SceNVSKitActivationData { // size is 0x20 bytes
	char magic[4]; // "act\n"
	uint32_t issue_no;
	uint32_t end_date;
	uint32_t start_date;
	char cmac_hash[0x10];
} SceNVSKitActivationData;

const uint32_t act_rsa_n[] = {
	0x47245239, 0xDD3EBC17, 0x681E0577, 0x981B7CFB,
	0x7179289B, 0x20C2642A, 0x893B93C6, 0x3FFEA5A3,
	0x9FA212A0, 0xB62BDC64, 0x434B549B, 0xFC2A3E63,
	0x6FD74277, 0x5BD4041A, 0xC7A3E8A6, 0x5181E51C,
	0xB28E037C, 0xD5847215, 0xDE2713E6, 0x5A79B0C8,
	0x5B424263, 0x1E871E47, 0x819F5E31, 0x1C8AF859,
	0x16B249B6, 0x58F71FDD, 0xB3EB113A, 0xD9B65FA6,
	0x76CAF6FC, 0x9BCE59FC, 0xF74CBC17, 0x28DBCD99,
	0x66EA6EE2, 0x84DC3999, 0x19395124, 0x6939E722,
	0x977D7A0C, 0x85F86C86, 0x77EFA707, 0xACB808C5,
	0x9CD1EC70, 0x4C2C8D2F, 0x3C6C3848, 0x971D5406,
	0x6060E96B, 0xFFC6CD6B, 0x8677A0AC, 0x9E1CC8DC,
	0x4469D58E, 0xE8B6AD26, 0x7B6BB7CE, 0x41466E16,
	0x96B1FFAF, 0xBB18B5A5, 0x2226B027, 0x92BBA884,
	0xB5D9610E, 0x55DCDD07, 0xC3602643, 0x431AFECF,
	0xFF257FA6, 0x113FA511, 0x308B55E9, 0xA844CC0E
};

const char act_aes_key_key[] = {
	0x84, 0x6D, 0x2D, 0xFD, 0x77, 0xD3, 0xC2, 0xE5, 0xF0, 0xE1, 0x7E, 0xB1, 0x8C, 0xC7, 0x86, 0x92,
	0x8B, 0x88, 0x1E, 0x2E, 0x17, 0xAE, 0x0C, 0xD8, 0xFD, 0xE8, 0x88, 0x09, 0xD0, 0xD0, 0x33, 0xC5
};

const char act_aes_key_iv[] = {
	0xC8, 0xA0, 0x40, 0x66, 0x2B, 0x10, 0xA1, 0x98, 0x6A, 0x18, 0x94, 0xE9, 0x4F, 0xBE, 0xFC, 0xF0
};

// for NVS act data
const char act_aes_cmac_key[] = {
	0x5A, 0x91, 0xFC, 0x74, 0xA8, 0x2B, 0xE3, 0xF2, 0xB8, 0xF4, 0xDB, 0x60, 0x70, 0xA0, 0x99, 0xA2,
	0xBD, 0xF0, 0x0E, 0x7B, 0xF0, 0x0E, 0x7B, 0xF0, 0x8B, 0x68, 0x55, 0x34, 0xA0, 0x64, 0x6D, 0x87
};

const SceNVSKitActivationData NVS_ACT_BLOCK = {
	.magic      = {
		'a', 'c', 't', 0
	},
	.issue_no   = 2,
	.end_date   = 0x5381D66E,
	.start_date = 0x530B2F6E,
	.cmac_hash  = {
		0x39, 0xF7, 0x59, 0x2E, 0x21, 0x7E, 0xB4, 0xA6, 0x6D, 0xDD, 0x9E, 0xE3, 0xF1, 0x46, 0x98, 0x3F
	}
};

char actsig[0x100] __attribute__((aligned(0x40)));
SceKitActivationData act_data __attribute__((aligned(0x40)));
SceNVSKitActivationData nvs_act_data;

int __swap_data32(uint32_t *dst, const uint32_t *src, SceSize len){

	uint32_t val;
	int s1 = ((len & 4) != 0) ? 1 : 0;

	for(int i=0;i<(len >> 3);i++){
		val = __builtin_bswap32(src[(len >> 2) - i - 1]);
		dst[(len >> 2) - i - 1] = __builtin_bswap32(src[i]);
		dst[i] = val;
	}

	if(s1 != 0)
		dst[len >> 3] = __builtin_bswap32(src[len >> 3]);

	return 0;
}

int actVerifierCheckSystemFw(void){

	int res;
	SceKernelFwInfo fw_data;
	memset(&fw_data, 0, sizeof(fw_data));
	fw_data.size = sizeof(fw_data);

	res = ksceKernelGetSystemSwVersion(&fw_data);
	if(res < 0)
		return res;

	if(((fw_data.version & ~0xFFF) - 0x3600000) >= 0x140000)
		return -1;

	return 0;
}

int actVerifierReadActData(void){

	SceUID fd;

	fd = ksceIoOpen("tm0:activate/act.dat", SCE_O_RDONLY, 0);
	if(fd < 0)
		return fd;

	ksceIoRead(fd, &act_data, sizeof(act_data));
	ksceIoClose(fd);

	fd = ksceIoOpen("tm0:activate/actsig.dat", SCE_O_RDONLY, 0);
	if(fd < 0)
		return fd;

	ksceIoRead(fd, actsig, sizeof(actsig));
	ksceIoClose(fd);

	return 0;
}

int actVerifierMain(SceSize args, void *argp){

	int (* sceNpDrmRsaModPower)(void *dst, const void *src, SceNpDrmRsaKey *pParam, int size);
	int res;

	res = actVerifierCheckSystemFw();
	if(res < 0)
		return res;

	SceUID SceNpDrm_moduleid = ksceKernelSearchModuleByName("SceNpDrm");
	if(SceNpDrm_moduleid < 0)
		return SceNpDrm_moduleid;

	res = module_get_offset(0x10005, SceNpDrm_moduleid, 0, 0xEDD4 | 1, (uintptr_t *)&sceNpDrmRsaModPower);
	if(res < 0)
		return res;

	res = actVerifierReadActData();
	if(res < 0)
		return res;

	__swap_data32((uint32_t *)actsig, (uint32_t *)actsig, 0x100);

	uint32_t act_rsa_e[0x40];
	memset(act_rsa_e, 0, sizeof(act_rsa_e));
	act_rsa_e[0] = 0x10001;

	SceNpDrmRsaKey param;
	param.n = act_rsa_n;
	param.k = act_rsa_e;

	sceNpDrmRsaModPower(actsig, actsig, &param, 0x40);

	__swap_data32((uint32_t *)actsig, (uint32_t *)actsig, 0x20);

	char hash[0x20];
	ksceSha256Digest(&act_data, sizeof(act_data), hash);

	if(memcmp(hash, actsig, 0x20) != 0){
		ksceDebugPrintf("RSA hash not matched\n");
		return -1;
	}

	char iv[0x10];
	memcpy(iv, act_aes_key_iv, sizeof(iv));

	ksceSblDmac5AesCbcDec(&act_data.enced, &act_data.enced, sizeof(act_data.enced), act_aes_key_key, 256, iv, 1);

	char mac[0x10];

	actVerifierAes256cmac(act_aes_key_key, (char *)&act_data, 0x70, mac);

	if(memcmp(act_data.enced.cmac, mac, 0x10) != 0){
		ksceDebugPrintf("Act data cmac not matched\n");
		return -1;
	}

	res = ksceSblNvsReadData(0x520, &nvs_act_data, 0x20);
	if(res < 0){
		ksceDebugPrintf("sceSblNvsReadData failed 0x%X\n", res);
		return res;
	}

	actVerifierAes256cmac(act_aes_cmac_key, (char *)&nvs_act_data, 0x10, mac);

	if(memcmp(nvs_act_data.cmac_hash, mac, 0x10) != 0){
		ksceDebugPrintf("NVS Act data cmac not matched\n");
		return -1;
	}

	ksceDebugPrintf("All ok\n");

	return 0;
}

int actVerifierMainExtendStack(void *argp){
	return actVerifierMain(4, argp);
}

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp){

	int res = ksceKernelRunWithStack(0x4000, actVerifierMainExtendStack, argp);

	return (res >= 0) ? SCE_KERNEL_START_NO_RESIDENT : SCE_KERNEL_START_FAILED;
}
