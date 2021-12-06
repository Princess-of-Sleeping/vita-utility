
#ifndef _MY_PSP2_PAF_H_
#define _MY_PSP2_PAF_H_

#include <psp2/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ScePafInitParam { // size is 0x18-bytes
	SceSize global_heap_size;
	SceUInt32 unk_0x04;
	SceUInt32 unk_0x08;
	SceBool is_cdialog_mode;
	SceSize heap_option_align;
	SceBool heap_option_unk;
} ScePafInitParam;

#ifdef __cplusplus
}
#endif

#endif /* _MY_PSP2_PAF_H_ */
