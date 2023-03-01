
#ifndef _PSP2_KIRK_H_
#define _PSP2_KIRK_H_

#ifdef __cplusplus
extern "C" {
#endif


#include <psp2kern/types.h>


typedef struct SceKirk_0x15_input {
	SceUInt8 scalar[0x1C];
	SceUInt8 x[0x1C];
	SceUInt8 y[0x1C];
} SceKirk_0x15_input;

typedef struct SceKirk_0x15_output {
	SceUInt8 x[0x1C];
	SceUInt8 y[0x1C];
} SceKirk_0x15_output;

typedef struct SceKirk_0x18_input {
	SceUInt8 x[0x1C];
	SceUInt8 y[0x1C];
	SceUInt8 hash[0x1C];
	SceUInt8 r[0x1C];
	SceUInt8 s[0x1C];
} SceKirk_0x18_input;


typedef struct SceSblSmCommGcData { // size is 0x814
	int unk_0; // 1
	int command;
	char data[0x800];
	int key_id;
	int size;
	int unk_810; // 0
} SceSblSmCommGcData;


#ifdef __cplusplus
}
#endif

#endif /* _PSP2_KIRK_H_ */
