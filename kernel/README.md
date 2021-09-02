# DevkitTV

A kernel plugin to update screen on/off from HDMI connection on devkit.

```
HDMI connected     -> screen off
HDMI disconnection -> screen on
```

# MappingVaddrSample

Sample of how to map vaddr from paddr

# SceDeci4pRDrfpUnlimited

Unlimited SceDeci4pRDrfp io file control patch.

For example, Can access to vs0: from host psp2ctrl commands.

# SceErrorHelper

Popup error dialog strings code to hex.

# SceHwbkptTest

Settings hardware break point on thread and process.

In the sample, the following code is executed in the application NPXS10015 (Settings).

```
Normal            : Opening to "Error history", trigger hardware break point.
If pressed start  : If called sceSettingsMain (SceSettings_text + 0x110), trigger hardware break point.
If pressed select : If access to settings_type variable (SceSettings_data + 0x3C), trigger hardware watch point.
```

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

# np_act_verifier

Verify SceNpDrm activation file.

# dipsw_show

Show current dipsw on tty.

# noSuspend

Disable suspend.

note

The screen goes off, but it doesn't suspend completely.

But ftp connection is not available with the screen off.

# noSuspendMini

Disable suspend.

note

The screen goes off, but it doesn't suspend completely.

Ftp connection is available with the screen off.

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

# malloc_free_test_kernel

Test of malloc/free implemented in Kernel

# nvs_dumper

Dumper for syscon non-volatile storage.

# slb2_dumper

Dumper for Secure loader bank.

But secure loader bank is guessed name.

# syscon_vs_dumper

Dumper for syscon vs storage.

vs storage includes resume context address, rtc tick, dipsw, etc.

# idstorage_dumper

Dumper for idstorage.

# idstorage_raw_dumper

Dumper for idstorage raw (partition level).

# kblparam_dumper

Dumper for SceKblParam (old name is sysroot).

# update_sw_list

update_mgr testing stuff

# visible_id_dumper

Dumper for Visible id (like console id).
