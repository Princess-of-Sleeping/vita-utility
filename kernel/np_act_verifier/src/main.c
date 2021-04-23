/*
 * PSVita np act verifier
 * Copyright (C) 2021, Princess of Sleeping
 */

#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/threadmgr.h>
#include <psp2kern/kernel/sysclib.h>
#include <psp2kern/kernel/utils.h>
#include <psp2kern/kernel/ssmgr.h>
#include <psp2kern/kernel/debug.h>
#include <psp2kern/io/fcntl.h>

int module_get_offset(SceUID pid, SceUID modid, int segidx, uint32_t offset, uintptr_t *dst);

typedef struct SceNpDrmRsaKey {
	const void *n;
	const void *k; // e/d
} SceNpDrmRsaKey;

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

int __swap_data32(uint32_t *dst, const uint32_t *src, SceSize len){

	uint32_t val;
	int s1 = ((len & 4) != 0) ? 1 : 0;

	for(int i=0;i<(len >> 3);i++){
		val = __builtin_bswap32(src[(len >> 2) - i - 1]);
		dst[(len >> 2) - i - 1] = __builtin_bswap32(src[i]);
		dst[i] = val;
	}

	if(s1 != 0)
		dst[len >> 3] = __builtin_bswap32(dst[len >> 3]);

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

char act_data[0x1038] __attribute__((aligned(0x40)));

int actVerifierReadActData(void){

	SceUID fd;

	fd = ksceIoOpen("host0:act.dat", SCE_O_RDONLY, 0);
	if(fd < 0)
		fd = ksceIoOpen("tm0:npdrm/act.dat", SCE_O_RDONLY, 0);
	if(fd < 0)
		return fd;

	ksceIoRead(fd, act_data, sizeof(act_data));
	ksceIoClose(fd);

	return 0;
}

int aes_dec(void *data, SceSize size, const void *key){

	int res;
	SceAesContext ctx;

	res = ksceAesInit1(&ctx, 0x80, 0x80, key);
	if(res < 0)
		return res;

	size = (size & ~(0x10 - 1)) >> 4;

	for(int i=0;i<size;i++){
		ksceAesDecrypt1(&ctx, data, data);
		data += 0x10;
	}

	res = 0;

	return res;
}

typedef struct SceNpDrmEcdsaParam { // size is 0xC
	void *hash;
	void *sign;
	int type;
} SceNpDrmEcdsaParam;

int actVerifierMain(SceSize args, void *argp){

	int (* sceNpDrmEcdsaVerify)(SceNpDrmEcdsaParam *pParam);
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

	res = module_get_offset(0x10005, SceNpDrm_moduleid, 0, 0xA024 | 1, (uintptr_t *)&sceNpDrmEcdsaVerify);
	if(res < 0)
		return res;

	res = actVerifierReadActData();
	if(res < 0)
		return res;

	char hash_sha1[0x14];
	ksceSha1Digest(act_data, 0x1010, hash_sha1);

	SceNpDrmEcdsaParam ecdsa_param;
	ecdsa_param.hash = hash_sha1;
	ecdsa_param.sign = act_data + 0x1010;
	ecdsa_param.type = 0;

	res = sceNpDrmEcdsaVerify(&ecdsa_param);
	ksceDebugPrintf("sceNpDrmEcdsaVerify:0x%X\n", res);

	const void *act_aes_key = NULL;
	const void *act_rsa_n = NULL;

	res = module_get_offset(0x10005, SceNpDrm_moduleid, 0, 0xFD64, (uintptr_t *)&act_rsa_n);
	if(res < 0)
		return res;

	res = module_get_offset(0x10005, SceNpDrm_moduleid, 1, 0x10, (uintptr_t *)&act_aes_key);
	if(res < 0)
		return res;

	__swap_data32((uint32_t *)(&act_data[0xED0]), (uint32_t *)(&act_data[0xED0]), 0x100);

	uint32_t act_rsa_e[0x40];
	memset(act_rsa_e, 0, sizeof(act_rsa_e));
	act_rsa_e[0] = 0x10001;

	SceNpDrmRsaKey param;
	param.n = act_rsa_n;
	param.k = act_rsa_e;

	sceNpDrmRsaModPower(&act_data[0xED0], &act_data[0xED0], &param, 0x40);

	__swap_data32((uint32_t *)&act_data[0xED0], (uint32_t *)&act_data[0xED0], 0x20);

	char hash[0x20];
	ksceSha256Digest(act_data, 0xED0, hash);

	if(memcmp(hash, &act_data[0xED0], 0x20) != 0){
		ksceDebugPrintf("RSA hash not matched\n");
		return -1;
	}

	aes_dec(&act_data[0x10], 0x800, act_aes_key);

	// write_file("host0:act_data.bin", act_data, sizeof(act_data));

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
