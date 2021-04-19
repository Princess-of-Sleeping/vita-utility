/*
 * SceZlib sample
 * Copyright (C) 2021 Princess of Sleeping
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/sysmem.h>
#include <psp2kern/kernel/debug.h>
#include <psp2kern/kernel/sysclib.h>
#include <psp2kern/kernel/iofilemgr.h>
#include <psp2kern/io/fcntl.h>
#include <taihen.h>

typedef void *(* SceZlibAllocFunc)(void *opaque, SceUInt items, SceSize size);
typedef void (* SceZlibFreeFunc)(void *opaque, void *address);

typedef struct SceZlibStream { // size is 0x38
	const void *next_in; //<! next input byte
	SceUInt32  avail_in; //<! number of bytes available at next_in
	SceUInt32  total_in; //<! total number of input bytes read so far

	void    *next_out; //<! next output byte should be put there
	SceSize avail_out; //<! remaining free space at next_out
	SceSize total_out; //<! total number of bytes output so far

	const char *msg;   //<! last error message, NULL if no error
	void       *state; //<! internal data. size is 0x16C4

	SceZlibAllocFunc zalloc; //<! used to allocate the internal state
	SceZlibFreeFunc  zfree;  //<! used to free the internal state
	void            *opaque; //<! private data object passed to zalloc and zfree

	SceInt32  data_type; //<! best guess about the data type: binary or text
	SceUInt32 adler;     //<! adler32 value of the uncompressed data
	SceUInt32 reserved;  //<! reserved for future use
} __attribute__((packed)) SceZlibStream;

int (* sceZlibDeflateInit)(SceZlibStream *strm, int a2, const char *zlib_ver, SceSize strm_size);
int (* SceZlibForDriver_20A122F8)(SceZlibStream *strm, int a2, int a3, int a4, int a5, int a6, const char *zlib_ver, SceSize strm_size);
int (* sceZlibDeflateEnd)(SceZlibStream *strm);
int (* sceZlibDeflate)(SceZlibStream *strm, int a2);

int module_get_export_func(SceUID pid, const char *modname, uint32_t lib_nid, uint32_t func_nid, uintptr_t *func);
#define GetExport(modname, lib_nid, func_nid, func) module_get_export_func(0x10005, modname, lib_nid, func_nid, (uintptr_t *)func)

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

SceUID heapid;
int zlib_args[0x8]; // fake

void *my_zlib_alloc(void *args, SceUInt32 num, SceSize size){
	return ksceKernelAllocHeapMemory(heapid, num * size);
}

void my_zlib_free(void *args, void *ptr){
	ksceKernelFreeHeapMemory(heapid, ptr);
}

char working_buffer[0x10000];
char zlib_buf[0x2000];

// Can you gzip open by 7z.
#define SCE_ZLIB_OUTPUT_GZIP (1)

int zlib_test(void){

	int res;

	GetExport("SceSblPostSsMgr", 0xE241534E, 0x25F28DA7, &sceZlibDeflateInit);
	GetExport("SceSblPostSsMgr", 0xE241534E, 0x20A122F8, &SceZlibForDriver_20A122F8);
	GetExport("SceSblPostSsMgr", 0xE241534E, 0x5492B3F2, &sceZlibDeflateEnd);
	GetExport("SceSblPostSsMgr", 0xE241534E, 0x5B718E55, &sceZlibDeflate);

	SceKernelHeapCreateOpt heap_opt;
	memset(&heap_opt, 0, sizeof(heap_opt));
	heap_opt.size = sizeof(heap_opt);
	heap_opt.attr = SCE_KERNEL_HEAP_ATTR_HAS_AUTO_EXTEND | SCE_KERNEL_HEAP_ATTR_HAS_MEMORY_TYPE;
	// heap_opt.memtype = 0x10F0D006;
	heap_opt.memtype = 0x1020D006;

	heapid = ksceKernelCreateHeap("SceZlibSampleHeap", 0x2000, &heap_opt);
	if(heapid < 0){
		ksceDebugPrintf("sceKernelCreateHeap failed : 0x%X\n", heapid);
		return -1;
	}

	SceZlibStream strm;
	memset(&strm, 0, sizeof(strm));

	strm.zalloc = my_zlib_alloc;
	strm.zfree  = my_zlib_free;
	strm.opaque = zlib_args;

#if SCE_ZLIB_OUTPUT_GZIP == 0
	res = sceZlibDeflateInit(&strm, 1, "1.2.5", sizeof(strm));
	ksceDebugPrintf("sceZlibDeflateInit:0x%X\n", res);
#else
	res = SceZlibForDriver_20A122F8(&strm, 1, 8, 0x18, 1, 0, "1.2.5", sizeof(strm));
	ksceDebugPrintf("SceZlibForDriver_20A122F8:0x%X\n", res);
#endif

	memset(zlib_buf, 0, sizeof(zlib_buf));
	snprintf(zlib_buf, sizeof(zlib_buf), "Hiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii, SceZlib Sample!!!\n");

	strm.next_in  = NULL;
	strm.avail_in = 0;
	strm.total_in = 0;
	strm.next_out  = working_buffer;
	strm.avail_out = sizeof(working_buffer);
	strm.total_out = 0;

	strm.next_in = zlib_buf;
	strm.avail_in = 0x1900;

	res = sceZlibDeflate(&strm, 0);
	ksceDebugPrintf("sceZlibDeflate:0x%X\n", res);

	strm.next_in = zlib_buf;
	strm.avail_in = 0x1800;

	res = sceZlibDeflate(&strm, 0);
	ksceDebugPrintf("sceZlibDeflate:0x%X\n", res);

	strm.next_in = zlib_buf;
	strm.avail_in = 0x100;

	res = sceZlibDeflate(&strm, 4);
	ksceDebugPrintf("sceZlibDeflate:0x%X\n", res);

	if(res == 1){
		char drv[8], ext[8], path[0x40];

#if SCE_ZLIB_OUTPUT_GZIP == 0
		snprintf(ext, sizeof(ext), "bin");
#else
		snprintf(ext, sizeof(ext), "gzip");
#endif

		SceIoStat stat;
		if(ksceIoGetstat("host0:", &stat) >= 0){
			snprintf(drv, sizeof(drv), "host0:");
		}else if(ksceIoGetstat("uma0:", &stat) >= 0){
			snprintf(drv, sizeof(drv), "uma0:");
		}else if(ksceIoGetstat("sd0:", &stat) >= 0){
			snprintf(drv, sizeof(drv), "sd0:");
		}else if(ksceIoGetstat("ux0:", &stat) >= 0){
			snprintf(drv, sizeof(drv), "ux0:");
		}else{
			drv[0] = 0;
		}

		if(drv[0] != 0){
			snprintf(path, sizeof(path), "%s/zlib_sample_output.%s", drv, ext);
			write_file(path, working_buffer, strm.total_out);
		}
	}

	res = sceZlibDeflateEnd(&strm);
	ksceDebugPrintf("sceZlibDeflateEnd:0x%X\n", res);

	ksceKernelDeleteHeap(heapid);
	heapid = -1;

	return 0;
}

int zlib_test_file(void){

	int res;

	GetExport("SceSblPostSsMgr", 0xE241534E, 0x25F28DA7, &sceZlibDeflateInit);
	GetExport("SceSblPostSsMgr", 0xE241534E, 0x20A122F8, &SceZlibForDriver_20A122F8);
	GetExport("SceSblPostSsMgr", 0xE241534E, 0x5492B3F2, &sceZlibDeflateEnd);
	GetExport("SceSblPostSsMgr", 0xE241534E, 0x5B718E55, &sceZlibDeflate);

	SceKernelHeapCreateOpt heap_opt;
	memset(&heap_opt, 0, sizeof(heap_opt));
	heap_opt.size    = sizeof(heap_opt);
	heap_opt.attr    = SCE_KERNEL_HEAP_ATTR_HAS_AUTO_EXTEND | SCE_KERNEL_HEAP_ATTR_HAS_MEMORY_TYPE;
	heap_opt.memtype = 0x10F0D006;

	heapid = ksceKernelCreateHeap("SceZlibSampleHeap", 0x2000, &heap_opt);
	if(heapid < 0){
		ksceDebugPrintf("sceKernelCreateHeap failed : 0x%X\n", heapid);
		return -1;
	}

	SceUID memid = ksceKernelAllocMemBlock("SceZlibMemblk", 0x10F0D006, 0x800000, NULL);
	if(memid < 0){
		ksceDebugPrintf("sceKernelAllocMemBlock failed : 0x%X\n", memid);
		goto del_heap;
	}

	void *zlib_output = NULL;
	ksceKernelGetMemBlockBase(memid, &zlib_output);

	memset(zlib_output, 0, 0x800000);

	SceZlibStream strm;
	memset(&strm, 0, sizeof(strm));

	strm.zalloc = my_zlib_alloc;
	strm.zfree  = my_zlib_free;
	strm.opaque = zlib_args;

#if SCE_ZLIB_OUTPUT_GZIP == 0
	res = sceZlibDeflateInit(&strm, 1, "1.2.5", sizeof(strm));
	ksceDebugPrintf("sceZlibDeflateInit:0x%X\n", res);
#else
	res = SceZlibForDriver_20A122F8(&strm, 1, 8, 0x18, 1, 0, "1.2.5", sizeof(strm));
	ksceDebugPrintf("SceZlibForDriver_20A122F8:0x%X\n", res);
#endif

	strm.next_in  = NULL;
	strm.avail_in = 0;
	strm.total_in = 0;
	strm.next_out  = zlib_output;
	strm.avail_out = 0x800000;
	strm.total_out = 0;

	int file_size, zlib_res = -1;

	SceUID fd = ksceIoOpen("sd0:os0.bin", SCE_O_RDONLY, 0);
	if(fd < 0){
		ksceDebugPrintf("sceIoOpen failed : 0x%X\n", fd);
		goto del_memblk;
	}

	SceIoStat stat;

	res = ksceIoGetstatByFd(fd, &stat);
	if(res < 0){
		ksceDebugPrintf("sceIoGetstatByFd failed : 0x%X\n", res);
		goto io_close;
	}

	file_size = (int)stat.st_size;

	do {
		memset(working_buffer, 0, sizeof(working_buffer));
		res = ksceIoRead(fd, working_buffer, sizeof(working_buffer));
		if(res > 0){
			file_size    -= res;
			strm.next_in  = working_buffer;
			strm.avail_in = res;

			zlib_res = sceZlibDeflate(&strm, (file_size != 0) ? 0 : 4);
		}
	} while(res > 0 && zlib_res == 0);

io_close:
	ksceIoClose(fd);

	if(zlib_res == 1){
		char drv[8], ext[8], path[0x40];

#if SCE_ZLIB_OUTPUT_GZIP == 0
		snprintf(ext, sizeof(ext), "bin");
#else
		snprintf(ext, sizeof(ext), "gzip");
#endif

		SceIoStat stat;
		if(ksceIoGetstat("host0:", &stat) >= 0){
			snprintf(drv, sizeof(drv), "host0:");
		}else if(ksceIoGetstat("uma0:", &stat) >= 0){
			snprintf(drv, sizeof(drv), "uma0:");
		}else if(ksceIoGetstat("sd0:", &stat) >= 0){
			snprintf(drv, sizeof(drv), "sd0:");
		}else if(ksceIoGetstat("ux0:", &stat) >= 0){
			snprintf(drv, sizeof(drv), "ux0:");
		}else{
			drv[0] = 0;
		}

		if(drv[0] != 0){
			snprintf(path, sizeof(path), "%s/zlib_sample_output.%s", drv, ext);
			write_file(path, zlib_output, strm.total_out);
		}
	}

	res = sceZlibDeflateEnd(&strm);
	ksceDebugPrintf("sceZlibDeflateEnd:0x%X\n", res);

del_memblk:
	ksceKernelFreeMemBlock(memid);
	memid = -1;

del_heap:
	ksceKernelDeleteHeap(heapid);
	heapid = -1;

	return 0;
}

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp){

	// zlib_test();
	zlib_test_file();

	return SCE_KERNEL_START_NO_RESIDENT;
}
