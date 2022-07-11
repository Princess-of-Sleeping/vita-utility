/*
 * SceNetPs sceNetRecvfrom kernel call 0xC0022005 fixer
 * Copyright (C) 2022, Princess of Sleeping
 */

#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/sysclib.h>
#include <psp2kern/kernel/utils.h>
#include <taihen.h>


int module_get_offset(SceUID pid, SceUID modid, int segidx, size_t offset, uintptr_t *addr);


const char point_sha256[0x20] = {
	0x3C, 0x48, 0x10, 0x8F, 0x3B, 0x53, 0x15, 0x94, 0x3B, 0x4A, 0xDB, 0x97, 0xD6, 0x9F, 0x2A, 0xCC,
	0x55, 0x9D, 0x21, 0x4B, 0x80, 0xC8, 0x95, 0xA0, 0x0D, 0xFD, 0x43, 0xB2, 0x3A, 0x11, 0x8B, 0x63
};

const char point_sha256_patched[0x20] = {
	0x63, 0x35, 0xA6, 0xB2, 0x6E, 0x41, 0x7D, 0xF5, 0x9B, 0xB2, 0x56, 0xCD, 0xD1, 0x2E, 0x2F, 0x65,
	0xFE, 0xEF, 0xA9, 0x73, 0xD7, 0x53, 0x44, 0x17, 0xEE, 0x39, 0xBB, 0x17, 0xE5, 0x72, 0x0C, 0xC5
};

/*
        810067b2 c0 ef 10 00     vmov.i32   d16,#0              -> DD F8 30 C0   ldr ip, [sp, #0x30]
        810067b6 19 68           ldr        r1, [r3]
        810067b8 a2 60           str        r2, [r4, #8]
        810067ba da f8 0c 30     ldr.w      r3, [sl, #0xc]

        810067be c4 e9 07 55     strd       r5, r5, [r4,#0x1c]  -> C4 E9 07 C5   strd ip, r5, [r4,#0x1c]
        810067c2 a5 61           str        r5, [r4, #0x18]

        810067c4 e3 60           str        r3, [r4, #0xc]
        810067c6 61 62           str        r1, [r4, #0x24]

        810067c8 c4 ed 04 0b     vstr.64    d16, [r4,#0x10]     -> C4 E9 04 55   strd r5, r5, [r4,#0x10]
*/

int do_patch(void){

	int res;
	SceUID module_id;
	void *patch_point;
	char inst[0x20], hash[0x20];

	module_id = ksceKernelSearchModuleByName("SceNetPs");
	if(module_id < 0){
		return module_id;
	}

	module_get_offset(0x10005, module_id, 0, 0x67b2, (uintptr_t *)&patch_point);

	memcpy(inst, patch_point, 0x1A);

	res = ksceSha256Digest(inst, 0x1A, hash);
	if(res < 0){
		return res;
	}

	if(memcmp(hash, point_sha256, 0x20) != 0){
		return -1; // unsupported fw
	}

	memcpy(&(inst[0x0]), (const char[4]){0xDD, 0xF8, 0x30, 0xC0}, 4);
	memcpy(&(inst[0xC]), (const char[4]){0xC4, 0xE9, 0x07, 0xC5}, 4);
	memcpy(&(inst[0x16]), (const char[4]){0xC4, 0xE9, 0x04, 0x55}, 4);

	res = ksceSha256Digest(inst, 0x1A, hash);
	if(res < 0){
		return res;
	}

	if(memcmp(hash, point_sha256_patched, 0x20) != 0){
		return -2; // something wrong
	}

	res = taiInjectDataForKernel(0x10005, module_id, 0, 0x67b2, inst, 0x1A);
	if(res < 0){
		return res;
	}

	return 0;
}

void _start() __attribute__ ((weak, alias ("module_start")));
int module_start(SceSize argc, const void *args){

	do_patch();

	return SCE_KERNEL_START_NO_RESIDENT;
}
