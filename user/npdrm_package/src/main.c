/*
 * SCE Package Decrypter
 * Copyright (C) 2023, Princess of Sleeping
 */

#include <psp2/kernel/modulemgr.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/clib.h>
#include <psp2/io/fcntl.h>


#define SCE_NPDRM_PACKAGE_CHECK_SIZE (0x8000)


int _sceNpDrmPackageCheck(const void *buffer, SceSize size, void *a3, int flags);

typedef struct SceNpDrmPackageDecryptParam {
	SceOff offset;
	int flags;
	int padding;
} SceNpDrmPackageDecryptParam;

int _sceNpDrmPackageDecrypt(void *buffer, SceSize size, const SceNpDrmPackageDecryptParam *pParam);




typedef struct ScePkgItemInfo { // size is 0x20-bytes
	SceSize file_name_offset;
	SceSize file_name_len;
	SceOff data_offset;
	SceOff data_size;
	SceUInt32 flags;
	SceUInt32 padding;
} ScePkgItemInfo;

SceUInt8 pkg_header[SCE_NPDRM_PACKAGE_CHECK_SIZE];
char pkg_ent_name[0x80];

int program_start(void){

	int res;
	SceUID fd;
	ScePkgItemInfo item;

	fd = sceIoOpen("host0:package/JP0741-PCSG90096_00-BITTERSMILEJPNDM.pkg", SCE_O_RDONLY, 0);
	if(fd < 0){
		return fd;
	}

	sceIoRead(fd, pkg_header, sizeof(pkg_header));

	do {
		res = _sceNpDrmPackageCheck(pkg_header, sizeof(pkg_header), NULL, 0x100);
		if(res < 0){
			sceClibPrintf("_sceNpDrmPackageCheck 0x%X\n", res);
			break;
		}

		int item_num = __builtin_bswap32(*(int *)(pkg_header + 0x14));
		SceOff encrypted_data_offset = __builtin_bswap64(*(SceOff *)(pkg_header + 0x20));

		for(int i=0;i<item_num;i++){
			sceIoLseek(fd, encrypted_data_offset + sizeof(item) * i, SCE_SEEK_SET);
			sceIoRead(fd, &item, sizeof(item));

			SceNpDrmPackageDecryptParam dec_param;
			dec_param.offset = sizeof(item) * i;
			dec_param.flags  = 0x100;

			res = _sceNpDrmPackageDecrypt(&item, sizeof(item), &dec_param);
			if(res < 0){
				sceClibPrintf("_sceNpDrmPackageDecrypt 0x%X\n", res);
				break;
			}

			SceUInt32 flags            = __builtin_bswap32(item.flags);
			SceUInt32 file_name_offset = __builtin_bswap32(item.file_name_offset);
			SceSize file_name_len      = __builtin_bswap32(item.file_name_len);
			SceUInt64 data_offset      = __builtin_bswap64(item.data_offset);
			SceUInt64 data_size        = __builtin_bswap64(item.data_size);

			sceClibMemset(pkg_ent_name, 0, sizeof(pkg_ent_name));

			sceIoLseek(fd, encrypted_data_offset + file_name_offset, SCE_SEEK_SET);
			sceIoRead(fd, pkg_ent_name, file_name_len);

			dec_param.offset = file_name_offset;
			dec_param.flags  = 0x100;

			res = _sceNpDrmPackageDecrypt(pkg_ent_name, file_name_len, &dec_param);
			if(res < 0){
				sceClibPrintf("_sceNpDrmPackageDecrypt 0x%X for name\n", res);
				break;
			}

			const char *type;

			switch(flags & 0x1F){
			case 3:
				type = "file";
				break;
			case 4:
				type = "dir";
				break;
			case 0xE:
				type = "SELF";
				break;
			case 0x11:
				type = "sce_pfs file";
				break;
			case 0x12:
				type = "sce_pfs dir";
				break;
			case 0x10:
			case 0x13:
			case 0x15:
			case 0x16:
			case 0x18:
				type = "sce_sys file";
				break;
			default:
				type = "unknown";
				break;
			}

			sceClibPrintf("%-15s flags 0x%08X offset 0x%08llX length 0x%08llX %s\n", type, flags, data_offset, data_size, pkg_ent_name);
		}
	} while(0);

	sceIoClose(fd);


	return 0;
}

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp){

	program_start();

	sceKernelExitProcess(0);

	return SCE_KERNEL_START_SUCCESS;
}
