
#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/debug.h>
#include <psp2kern/io/fcntl.h>
#include <taihen.h>


#ifndef TAI_CONTINUE_NEW
#define TAI_CONTINUE_NEW(__basefunc__, hook, ...) ({ \
  struct _tai_hook_user *cur, *next; \
  cur = (struct _tai_hook_user *)(hook); \
  next = (struct _tai_hook_user *)cur->next; \
  (next == NULL) ? \
    ((__typeof__(&__basefunc__))cur->old)(__VA_ARGS__) \
  : \
    ((__typeof__(&__basefunc__))next->func)(__VA_ARGS__) \
  ; \
})
#endif

SceUID hook_id = -1;


typedef struct _SceIoSchedulerExecuteIoCommand { // size is 0xB8-bytes
	int unk_0x00;
	int unk_0x04;
	int unk_0x08;
	int unk_0x0C;

	int unk_0x10;
	int unk_0x14;
	int unk_0x18;
	int unk_0x1C;

	int unk_0x20;
	SceUInt32 opcode;
	int unk_0x28;
	int unk_0x2C;

	SceUInt8 unk_0x30;
	SceUInt8 unk_0x31;
	SceUInt8 unk_0x32;
	SceUInt8 unk_0x33;
	SceUID pid;
	SceUInt32 unk_0x38;
	SceUID impersonateId;

	SceUInt32 unk_0x40;
	SceInt32 affinityMask;
	SceInt32 prio;
	SceUInt32 unk_0x4C;

	SceUInt32 unk_0x50;
	SceUInt32 unk_0x54;
	SceUInt32 unk_0x58;
	SceUInt32 unk_0x5C;

	SceUInt32 unk_0x60;

	union {
		struct {
			const char *path;
			int flags;
			int mode;
		} Open;
		struct {
			SceUID fd;
		} Close;
		struct {
			SceUID fd;
			void *data;
			SceSize length;
		} Read;
		struct {
			SceUID fd;
			const void *data;
			SceSize length;
		} Write;
		struct {
			SceUID fd;
			void *data;
			SceSize length;
			SceUInt32 unk_0x70;
			SceUInt32 unk_0x74;
			SceOff offset;
		} Pread;
		struct {
			SceUID fd;
			const void *data;
			SceSize length;
			SceUInt32 unk_0x70;
			SceUInt32 unk_0x74;
			SceOff offset;
		} Pwrite;
		struct {
			SceUID fd;
			int where;
			SceInt32 pad_0x6C;
			SceUInt32 offset_lo;
			SceInt32 offset_hi;
		} Lseek;
		struct {
			SceUID fd;
			int where;
			SceInt32 pad_0x6C;
			SceInt32 offset;
		} Lseek32;
		struct {
			const char *path;
			int flags;
		} Sync;
		struct {
			SceUID fd;
			int flags;
		} SyncByFd;
		struct {
			const char *path;
		} Remove;
		struct {
			const char *path;
			int mode;
		} Mkdir;
		struct {
			const char *path;
		} Rmdir;
		struct {
			const char *old_path;
			const char *new_path;
		} Rename;
		struct {
			SceUID fd;
			int cmd;
			const void *indata;
			SceSize inlen;
			void *outdata;
			SceSize outlen;
		} Ioctl;

		struct {
			const char *path;
		} Dopen;
		struct {
			SceUID fd;
		} Dclose;
		struct {
			SceUID fd;
			SceIoDirent *dir;
		} Dread;

		struct {
			const char *path;
			SceIoStat *stat;
			int bit;
		} Chstat;
		struct {
			SceUID fd;
			SceIoStat *stat;
			int bit;
		} ChstatByFd;

		struct {
			const char *path;
			SceIoStat *stat;
		} Stat;
		struct {
			SceUID fd;
			SceIoStat *stat;
		} StatByFd;

		struct {
			const char *dev;
			unsigned int cmd;
			void *indata;
			SceSize inlen;
			void *outdata;
			SceSize outlen;
		} Devctl;

		struct {
			SceUInt32 unk_0x64;
			SceUInt32 unk_0x68;
			SceUInt32 unk_0x6C;
			SceUInt32 unk_0x70;
			SceUInt32 unk_0x74;
			SceUInt32 unk_0x78;
			SceUInt32 unk_0x7C;
			SceUInt32 unk_0x80;
		} Raw;
	};

	SceUInt32 unk_0x84;
	SceUInt32 unk_0x88;
	SceUID fd;
	SceUInt32 unk_0x90;
	SceUInt32 unk_0x94;
	SceUInt32 unk_0x98;
	SceUInt32 unk_0x9C;
	SceUInt32 unk_0xA0;
	SceUInt32 unk_0xA4;
	SceUInt32 unk_0xA8;
	SceUInt32 unk_0xAC;
	SceUInt64 systemTime;
} __attribute__((packed)) SceIoSchedulerExecuteIoCommand;

tai_hook_ref_t sceIoSchedulerExecuteIoCommand_ref;
SceInt64 sceIoSchedulerExecuteIoCommand_patch(SceIoSchedulerExecuteIoCommand *pCommand){

	SceInt64 res;

	res = TAI_CONTINUE_NEW(sceIoSchedulerExecuteIoCommand_patch, sceIoSchedulerExecuteIoCommand_ref, pCommand);

	switch(pCommand->opcode){
	case 0x1:
		ksceKernelPrintf("Scheduler 0x%08llX sceIoOpen %s 0x%X 0%03o\n", res, pCommand->Open.path, pCommand->Open.flags, pCommand->Open.mode);
		break;
	case 0x2:
		ksceKernelPrintf("Scheduler 0x%08llX sceIoClose 0x%08X\n", res, pCommand->Close.fd);
		break;
	case 0x3:
		break;
	case 0x4:
		break;
	case 0x5:
		ksceKernelPrintf("Scheduler 0x%08llX sceIoLseek 0x%08X 0x%08X%08X %d\n", res, pCommand->Lseek.fd, pCommand->Lseek.offset_hi, pCommand->Lseek.offset_lo, pCommand->Lseek.where);
		break;
	case 0x6:
		ksceKernelPrintf("Scheduler 0x%08llX sceIoLseek32 0x%08X 0x%016llX %d\n", res, pCommand->Lseek32.fd, pCommand->Lseek32.offset, pCommand->Lseek32.where);
		break;
	case 0x7:
		ksceKernelPrintf(
			"Scheduler 0x%08llX sceIoIoctl 0x%08X 0x%X %p 0x%08X %p 0x%08X\n",
			res,
			pCommand->Ioctl.fd,
			pCommand->Ioctl.cmd,
			pCommand->Ioctl.indata,
			pCommand->Ioctl.inlen,
			pCommand->Ioctl.outdata,
			pCommand->Ioctl.outlen
		);
		break;
	case 0x8:
		ksceKernelPrintf("Scheduler 0x%08llX sceIoRemove %s\n", res, pCommand->Remove.path);
		break;
	case 0x9:
		ksceKernelPrintf("Scheduler 0x%08llX sceIoDopen %s\n", res, pCommand->Dopen.path);
		break;
	case 0xA:
		ksceKernelPrintf("Scheduler 0x%08llX sceIoDclose 0x%08X\n", res, pCommand->Dclose.fd);
		break;
	case 0xB:
		ksceKernelPrintf("Scheduler 0x%08llX sceIoDread 0x%08X %p\n", res, pCommand->Dread.fd, pCommand->Dread.dir);
		break;
	case 0xC:
		ksceKernelPrintf("Scheduler 0x%08llX sceIoMkdir %s 0%03o\n", res, pCommand->Mkdir.path, pCommand->Mkdir.mode);
		break;
	case 0xD:
		ksceKernelPrintf("Scheduler 0x%08llX sceIoRmdir %s\n", res, pCommand->Rmdir.path);
		break;
	case 0xE:
		ksceKernelPrintf("Scheduler 0x%08llX sceIoRename %s %s\n", res, pCommand->Rename.old_path, pCommand->Rename.new_path);
		break;
	case 0xF:
		ksceKernelPrintf("Scheduler 0x%08llX sceIoChstat %s %p 0x%X\n", res, pCommand->Chstat.path, pCommand->Chstat.stat, pCommand->Chstat.bit);
		break;
	case 0x10:
		ksceKernelPrintf("Scheduler 0x%08llX sceIoChstatByFd 0x%08X %p 0x%X\n", res, pCommand->ChstatByFd.fd, pCommand->ChstatByFd.stat, pCommand->ChstatByFd.bit);
		break;
	case 0x11:
		ksceKernelPrintf("Scheduler 0x%08llX sceIoGetStat %s %p\n", res, pCommand->Stat.path, pCommand->Stat.stat);
		break;
	case 0x12:
		ksceKernelPrintf("Scheduler 0x%08llX sceIoGetStatByFd 0x%08X %p\n", res, pCommand->StatByFd.fd, pCommand->StatByFd.stat);
		break;
	case 0x13:
		ksceKernelPrintf(
			"Scheduler 0x%08llX sceIoDevctl %s 0x%X %p 0x%08X %p 0x%08X\n",
			res,
			pCommand->Devctl.dev,
			pCommand->Devctl.cmd,
			pCommand->Devctl.indata,
			pCommand->Devctl.inlen,
			pCommand->Devctl.outdata,
			pCommand->Devctl.outlen
		);
		break;
	case 0x14:
		ksceKernelPrintf("Scheduler 0x%08llX sceIoSync %s 0x%X\n", res, pCommand->Sync.path, pCommand->Sync.flags);
		break;
	case 0x15:
		ksceKernelPrintf("Scheduler 0x%08llX sceIoSyncByFd 0x%08X 0x%X\n", res, pCommand->SyncByFd.fd, pCommand->SyncByFd.flags);
		break;
	case 0x16:
		break;
	case 0x17:
		break;
	case 0x18:
		ksceKernelPrintf("Scheduler 0x%08llX sceIoGetStat (0x18) %s %p\n", res, pCommand->Stat.path, pCommand->Stat.stat);
		break;
	case 0x19:
		ksceKernelPrintf("Scheduler 0x%08llX sceIoChstat (0x19) %s %p 0x%X\n", res, pCommand->Chstat.path, pCommand->Chstat.stat, pCommand->Chstat.bit);
		break;
	case 0x1A:
		ksceKernelPrintf("Scheduler 0x%08llX sceIoDread (0x1A) 0x%08X %p\n", res, pCommand->Dread.fd, pCommand->Dread.dir);
		break;
	case 0x1B:
		ksceKernelPrintf("Scheduler 0x%08llX sceIoDopenInternal %s\n", res, pCommand->Dopen.path);
		break;
	case 0x1C:
		ksceKernelPrintf("Scheduler 0x%08llX sceIoDreadInternal 0x%08X %p\n", res, pCommand->Dread.fd, pCommand->Dread.dir);
		break;
	case 0x1D:
		ksceKernelPrintf("Scheduler 0x%08llX sceIoDcloseInternal 0x%08X\n", res, pCommand->Dclose.fd);
		break;
	default:
		ksceKernelPrintf("unknown opcode 0x%X\n", pCommand->opcode);
		break;
	}

	return res;
}

void _start() __attribute__((weak, alias ("module_start")));
int module_start(SceSize argc, const void *args){

	SceUID moduleId = ksceKernelSearchModuleByName("SceIofilemgr");

	hook_id = taiHookFunctionOffsetForKernel(
		SCE_KERNEL_PROCESS_ID,
		&sceIoSchedulerExecuteIoCommand_ref,
		moduleId,
		0,
		0x1a078, // 3.72
		SCE_TRUE,
		sceIoSchedulerExecuteIoCommand_patch
	);

	return SCE_KERNEL_START_SUCCESS;
}
