/*
 * PSVita act verifier
 * Copyright (C) 2021, Princess of Sleeping
 */

#include <psp2kern/kernel/sysclib.h>
#include <psp2kern/kernel/utils.h>
#include "aes256cmac.h"

const char subkey_base[0x10] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x87
};

void actVerifierAes256cmacShiftLOne(const unsigned char *src, unsigned char *dst){

	unsigned char val, carry = 0;

	for(int i=0xF;i>=0;i--){
		val    = src[i];
		dst[i] = (val << 1) | carry;
		carry  = (val >> 7) & 1;
	}
}

void actVerifierAes256cmacXor(const void *a, const void *b, void *out) {
	((uint32_t *)out)[0] = ((uint32_t *)a)[0] ^ ((uint32_t *)b)[0];
	((uint32_t *)out)[1] = ((uint32_t *)a)[1] ^ ((uint32_t *)b)[1];
	((uint32_t *)out)[2] = ((uint32_t *)a)[2] ^ ((uint32_t *)b)[2];
	((uint32_t *)out)[3] = ((uint32_t *)a)[3] ^ ((uint32_t *)b)[3];
}

void actVerifierAes256cmacPadding(const void *src, unsigned char *pad, SceSize length){

	memcpy(pad, src, length);

	pad[length] = 0x80;
	memset(&pad[length + 1], 0, 0xF - length);
}

void actVerifierAes256cmacGenSubKey(SceAesContext *ctx, unsigned char *subkey1, unsigned char *subkey2) {

	unsigned char tmp[0x10];

	memset(tmp, 0, sizeof(tmp));

	ksceAesEncrypt1(ctx, tmp, tmp);

	actVerifierAes256cmacShiftLOne(tmp, subkey1);

	if((tmp[0] & 0x80) != 0){
		actVerifierAes256cmacXor(subkey1, subkey_base, subkey1);
	}

	actVerifierAes256cmacShiftLOne(subkey1, subkey2);

	if((subkey1[0] & 0x80) != 0){
		actVerifierAes256cmacXor(subkey2, subkey_base, subkey2);
	}
}

void actVerifierAes256cmac(const void *key, const void *src, SceSize length, void *cmac){

	const unsigned char *pInput = (const unsigned char *)src;
	unsigned char X[0x10], M[0x10], subkey1[0x10], subkey2[0x10];
	int n, flag;

	SceAesContext ctx;
	memset(&ctx, 0, sizeof(ctx));

	ksceAesInit1(&ctx, 128, 256, key);

	actVerifierAes256cmacGenSubKey(&ctx, subkey1, subkey2);

	flag = 0;
	n = (length + 0xF) >> 4;
	if(n != 0){
		flag = ((length & 0xF) == 0) ? 1 : 0;
		n--;
	}

	if(flag != 0){
		actVerifierAes256cmacXor((const void *)(((uintptr_t)src) + (n << 4)), subkey1, M);
	}else{
		actVerifierAes256cmacPadding((const void *)(((uintptr_t)src) + (n << 4)), subkey2, length & 0xF);
		actVerifierAes256cmacXor(subkey2, subkey2, M);
	}

	memset(X, 0, 0x10);
	for(int i=0;i<n;i++){
		actVerifierAes256cmacXor(X, pInput, X);
		ksceAesEncrypt1(&ctx, X, X);
		pInput = (const unsigned char *)(((uintptr_t)pInput) + 0x10);
	}

	actVerifierAes256cmacXor(X, M, X);
	ksceAesEncrypt1(&ctx, X, X);

	memcpy(cmac, X, 0x10);
}
