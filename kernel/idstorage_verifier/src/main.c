
#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/threadmgr.h>
#include <psp2kern/kernel/debug.h>
#include <psp2kern/kernel/sysroot.h>
#include <psp2kern/kernel/sysclib.h>
#include <psp2kern/kernel/ssmgr.h>
#include <psp2kern/kernel/utils.h>
#include <psp2kern/idstorage.h>
#include "kirk.h"
#include "cert.h"



typedef struct SceSblSmCommContext130 {
	uint32_t unk_0;
	uint32_t self_type;                    // kernel = 0, user = 1, sm = 2
	SceSelfAuthInfo caller_self_auth_info; // can be obtained with sceKernelGetSelfInfoForKernel
	SceSelfAuthInfo called_self_auth_info; // set by F00D in F00D SceSblSmCommContext130 response
	int path_id;                           // can be obtained with sceSblACMgrGetPathIdForKernel or sceIoGetPathIdExForDriver
	uint32_t unk_12C;
} SceSblSmCommContext130;

typedef struct SceSblSmCommPair {
	int data_00;
	int data_04;
} SceSblSmCommPair;

int ksceSblSmCommStartSmFromFile(int priority, const char *sm_path, int cmd_id, SceSblSmCommContext130 *ctx130, int *id);
int ksceSblSmCommStartSmFromData(int priority, const void *sm_data, SceSize sm_size, int cmd_id, SceSblSmCommContext130 *ctx130, int *id);

int ksceSblSmCommCallFunc(int id, int service_id, int *f00d_resp, void *data, SceSize size);
int ksceSblSmCommStopSm(int id, SceSblSmCommPair *result);

int ksceSblACMgrGetPathId(const char *path, int *pathId);

const SceSelfAuthInfo sm_selfinfo = {
	.program_authority_id = 0x2808000000000001,
	.padding = {0, 0, 0, 0, 0, 0, 0, 0},
	.capability = {
		0x80, 0x00, 0x00, 0x00, 0xC0, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	},
	.attribute = {
		0x80, 0x09, 0x80, 0x03, 0x00, 0x00, 0xC3, 0x00, 0x00, 0x00, 0x80, 0x09, 0x80, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF
	},
	.secret = {
		.shared_secret_0 = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		.klicensee       = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		.shared_secret_2 = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		.shared_secret_3_0 = 0,
		.shared_secret_3_1 = 0,
		.shared_secret_3_2 = 0,
		.shared_secret_3_3 = 0
	}
};

const SceUInt8 zeros[0x10] = {
};

const SceUInt8 kirk_0x17_aes_key[0x10] = {
	// Fill here by your key. this key is per-console.
};

const SceUInt8 Gx_224[0x1C] = {
	0x7E, 0x06, 0x09, 0x82, 0x47, 0xE6, 0xB5, 0x9F, 0x31, 0x10, 0xBC, 0xBB, 0x3A, 0xB6, 0xC2, 0x50,
	0xBC, 0x5A, 0xB0, 0x6C, 0x03, 0x2D, 0xAD, 0x43, 0x68, 0x4C, 0x24, 0x8F
};

const SceUInt8 Gy_224[0x1C] = {
	0x0B, 0xD9, 0x41, 0x8D, 0xE8, 0xE3, 0xE4, 0x5D, 0x2D, 0x70, 0x1E, 0x02, 0x37, 0xFD, 0x7F, 0x2A,
	0xDE, 0x0D, 0x48, 0xB7, 0x4C, 0xEE, 0xF2, 0xF1, 0xC8, 0xAC, 0x48, 0x4E
};

SceIdStoragePspCertificates psp_cert;
SceIdStoragePsp2Certificates psp2_cert;


int idstorage_read_vector(SceUInt32 leaf_start, void *leaf_data, SceUInt32 leaf_size){

	int res;

	for(int i=0;i<leaf_size;i++){
		res = ksceIdStorageReadLeaf(leaf_start + i, leaf_data + 0x200 * i);
		if(res < 0){
			return res;
		}
	}

	return 0;
}

int gcauthmgr_sm(void *result, int command, const void *data, int data_size, int key_id){

	int res, sm_id, f00d_resp;
	SceKernelSysrootSelfInfo sm_info;
	SceSblSmCommContext130 ctx130;
	SceSblSmCommGcData sm_arg;

	sm_info.size = sizeof(sm_info);

	res = ksceSysrootGetSelfInfo(SCE_KERNEL_SYSROOT_SELF_INDEX_GCAUTHMGR_SM, &sm_info);
	if(res < 0){
		return res;
	}

	memset(&ctx130, 0, sizeof(ctx130));
	memcpy(&(ctx130.caller_self_auth_info), &sm_selfinfo, sizeof(sm_selfinfo));

	ctx130.path_id = 2;
	ctx130.self_type = 2;

	res = ksceSblSmCommStartSmFromData(0, sm_info.self_data, sm_info.self_size, 0, &ctx130, &sm_id);
	if(res < 0){
		return res;
	}

	sm_arg.unk_0 = 1;
	sm_arg.command = command;
	sm_arg.key_id = key_id;
	memcpy(sm_arg.data, data, data_size);
	sm_arg.size = data_size;
	sm_arg.unk_810 = 0;

	res = ksceSblSmCommCallFunc(sm_id, 0x1000B, &f00d_resp, &sm_arg, sizeof(sm_arg));
	if(res < 0){
		f00d_resp = res;
	}

	if(result != NULL){
		if(sm_arg.command == 0x15){
			memcpy(result, sm_arg.data, sm_arg.size);
		}
	}

	SceSblSmCommPair sm_result;
	sm_result.data_00 = -1;
	sm_result.data_04 = -1;
	ksceSblSmCommStopSm(sm_id, &sm_result);

	return f00d_resp;
}

int verify_ecdsa_priv_key(ids_cert_psp2 *cert){

	int res;
	char iv[0x10];
	SceKirk_0x15_input input;
	SceKirk_0x15_output output;

	memset(iv, 0, sizeof(iv));

	res = ksceSblDmac5AesCbcDec(cert->cert_data.enc_priv_key, cert->cert_data.enc_priv_key, 0x20, kirk_0x17_aes_key, 128, iv, 1);
	if(res < 0){
		ksceKernelPrintf("sceSblDmac5AesCbcDec 0x%X\n", res);
		return res;
	}

	memcpy(input.scalar, cert->cert_data.enc_priv_key, 0x1C);
	memcpy(input.x, Gx_224, 0x1C);
	memcpy(input.y, Gy_224, 0x1C);

	res = gcauthmgr_sm(&output, 0x15, &input, 0x1C * 3, 0);
	if(res < 0){
		ksceKernelPrintf("gcauthmgr_sm 0x%X\n", res);
		return res;
	}

	if(memcmp(&(cert->cert_data.pub_key[0x00]), output.x, 0x1C) != 0){
		ksceKernelPrintf("cert pubkey->x does not match (%d)\n", 0);
	}

	if(memcmp(&(cert->cert_data.pub_key[0x1C]), output.y, 0x1C) != 0){
		ksceKernelPrintf("cert pubkey->y does not match (%d)\n", 0);
	}

	ksceKernelPrintf("cert private key is fine\n");

	return 0;
}

int verify_ecdsa_signature(ids_cert_psp2 *cert){

	int res;
	SceKirk_0x18_input input;

	memcpy(input.x, &(cert->cert_data.constant_pub_key[0x00]), 0x1C);
	memcpy(input.y, &(cert->cert_data.constant_pub_key[0x1C]), 0x1C);
	ksceSha224Digest(&(cert->cert_data), 0x48, input.hash);
	memcpy(input.r, &(cert->cert_data.signature.r), 0x1C);
	memcpy(input.s, &(cert->cert_data.signature.s), 0x1C);

	res = gcauthmgr_sm(NULL, 0x18, &input, sizeof(input), 0);
	if(res < 0){
		ksceKernelPrintf("failed verify signature 0x%X\n", res);
		return res;
	}

	if(res == 0){
		ksceKernelPrintf("cert signature is valid\n");
	}else{
		ksceKernelPrintf("cert signature is invalid\n");
	}

	return 0;
}

int verify_cert_cmac(ids_cert_psp2 *cert){

	int res;

	res = gcauthmgr_sm(NULL, 0x19, cert, sizeof(*cert), 0);
	if(res < 0){
		ksceKernelPrintf("failed cert cmac 0x%X\n", res);
		return res;
	}

	if(res == 0){
		ksceKernelPrintf("cert cmac is valid\n");
	}else{
		ksceKernelPrintf("cert cmac is invalid\n");
	}

	return 0;
}

int verify_psp2_cert(ids_cert_psp2 *cert){

	verify_cert_cmac(cert);

	if(memcmp(kirk_0x17_aes_key, zeros, 0x10) != 0){
		verify_ecdsa_priv_key(cert);
	}

	verify_ecdsa_signature(cert);

	return 0;
}

int cert_verify_entry(void *argp){

	int res;

	res = idstorage_read_vector(0, &psp_cert, 8);
	if(res < 0){
		ksceKernelPrintf("idstorage_read_vector 0x%X\n", res);
	}

	res = idstorage_read_vector(0x40, &psp2_cert, 8);
	if(res < 0){
		ksceKernelPrintf("idstorage_read_vector 0x%X\n", res);
	}

	for(int i=0;i<6;i++){
		verify_psp2_cert(&(psp2_cert.psp2[i]));
	}

	return 0;
}

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp){

	ksceKernelRunWithStack(0x2000, cert_verify_entry, NULL);

	return SCE_KERNEL_START_NO_RESIDENT;
}
