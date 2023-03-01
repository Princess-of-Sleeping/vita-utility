
#ifndef _PSP2_CERT_H_
#define _PSP2_CERT_H_

#ifdef __cplusplus
extern "C" {
#endif


#include <psp2kern/types.h>


typedef struct ECDSA160_signature { // size is 0x28
	unsigned char r[0x14];
	unsigned char s[0x14];
} ECDSA160_signature;

typedef struct ECDSA224_signature { // size is 0x38
	unsigned char r[0x1C];
	unsigned char s[0x1C];
} ECDSA224_signature;

typedef struct { // size is 0xA8
	char data[0x10];
	char pub_key[0x28]; // ?generated using KIRK 0xC? sent to KIRK 0x11 for verification
	ECDSA160_signature signature;
	char constant_pub_key[0x28]; // hardcoded constant, same in ALL PSPs but depends on the certificate index
	char enc_priv_key[0x20]; // sent to KIRK 0x10 for verification
} ids_cert_main_psp;

typedef struct { // size is 0xD8
	char data[0x10];
	char pub_key[0x38]; // ?generated using KIRK 0x14? sent to KIRK 0x18 for verification
	ECDSA224_signature signature;
	char constant_pub_key[0x38]; // hardcoded constant, same in ALL PSVitas but depends on the certificate index
	char enc_priv_key[0x20]; // sent to KIRK 0x17 for verification
} ids_cert_main_psp2;


typedef struct { // size is 0xB8
	ids_cert_main_psp cert_data; // data input for generating enc_aes_cmac_hash
	char aes_cmac[0x10];
} ids_cert_psp;

typedef struct { // size is 0xB8
	ids_cert_main_psp cert_data; // data input for generating enc_aes_cmac_hash
	char enc_aes_cmac[0x10]; // encrypted with per-section seed and master key
} ids_cert_ps3;

typedef struct { // size is 0xE8
	ids_cert_main_psp2 cert_data; // data for enc_aes_cmac_hash
	char enc_aes_cmac[0x10];      // encrypted with a ?per-section? seed and master bigmac keyslot 0x204
} ids_cert_psp2;


typedef struct { // size is 0xC0
	ids_cert_ps3 plain_cert; // encrypted using per-section seed
	char padding[0x10]; // zeroes
} enc_ids_cert_ps3;


typedef struct { // size is 0x488
	char unk_0[0x38]; // unrelated
	ids_cert_psp psp_certs[6];
} ids_certified_leaves_psp;

typedef struct {
	ids_cert_ps3 ps3_certs[12];
	ids_cert_psp2 psp2_certs[6];
} ids_certified_leaves_psp2;

typedef struct {
	enc_ids_cert_ps3 enc_ps3_certs[11];
} ids_certified_leaves_ps3;

// From leaf 0 to 7
typedef struct SceIdStoragePspCertificates {
	char unk_0x00[0x38];
	ids_cert_psp ids[6];   // 0-4 is cid, 5 is OpenPSID
	int  unk_488[2];       // 0, 0x78
	char unk_0x490[0x20];  // Random data? confirmation is necessary.
	int  unk_0x4A0[0x54];  // Some data array.

	// leaf 0x3 start
	char unk_0x600[0x1E0]; // Some data array.
	char unk_0x7E0[0x20];  // Random data? confirmation is necessary.

	char unk_0x800[0x200];
	char unk_0xA00[0x200];
	char unk_0xC00[0x200];
	char unk_0xE00[0x200];
} SceIdStoragePspCertificates;

// From leaf 0x40 to 0x47
typedef struct SceIdStoragePsp2Certificates { // size is 0x1000 byte
	ids_cert_ps3  ps3[6];
	ids_cert_ps3  psp[6];
	ids_cert_psp2 psp2[6];
	char unk_0xE10[0x1F0];
} SceIdStoragePsp2Certificates;


#ifdef __cplusplus
}
#endif

#endif /* _PSP2_CERT_H_ */
