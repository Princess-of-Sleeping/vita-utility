# DevkitTV

A kernel plugin to update screen on/off from HDMI connection.

```
HDMI connected     -> screen off
HDMI disconnection -> screen on
```

Known bug

There is a bug that the screen turns on without permission.

# MappingVaddrSample

Sample of how to map vaddr from paddr

# ScePfsMgrAllowDbgPfsPatch

Patch to allow debug version sce_pfs app mount and pkg install. 

# SceUsbstorVstorFat16

Supports FAT16 with usb connection.

# SceVfsMountPoC

Create host0 as a virtual device.

On devkit, mount to `vsd0:`. but non-openable file. something is different in retails and devkit.

The files that exist in "host0:kernel_modules/" and "host0:sysroot/" are RW.

__Since SceVfsMountPoC is a PoC, there is nothing that guarantees its operation.__

TODO : update createFileEntry and vfs_get_stat, support to devkit.

# SceZlibSample

Kernel zlib usage sample.

# KernelCustomMalloc

Malloc with custom memory base

Basically the same as sceKernelAllocHeapMemory

# CorelockSample

The Corelock Sample.

# act_verifier

Verify DevKit/TestingKit activation file.

# dipsw_show

Show current dipsw on tty.

# noSuspend

Disable suspend.

note

The screen goes off, but it doesn't suspend completely.

But ftp connection is not available with the screen off.

# map_user_memblk_by_paddr

Mapping userland memory block by any paddr.

It is also possible to refer to kernel memory directly from userland.

# kpanic_debug

Switch dbgFingerprint display to module name display.

note

To view the kpanic log, you need Princess Log or an equivalent logger.

Currently there is no more functionality.

# qaf_host0

Allow host0: access from module load etc.

# qaf_show

Show current qaf on tty.

# mac_address_spoofer

A PS Vita Mac address spoofer on kernel level working.

# nvs_dumper

Dumper for syscon non-volatile storage.

# syscon_vs_dumper

Dumper for syscon vs storage.

vs storage includes resume context address, rtc tick, dipsw, etc.

# idstorage_dumper

Dumper for idstorage.

# kblparam_dumper

Dumper for SceKblParam(old name is sysroot).
