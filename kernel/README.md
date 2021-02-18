# MappingVaddrSample

Sample of how to map vaddr from paddr

# ScePfsMgrAllowDbgPfsPatch

Patch to allow debug version sce_pfs app mount and pkg install. 

# SceVfsMountPoC

Create host0 as a virtual device.

The files that exist in "host0:kernel_modules/" and "host0:sysroot/" are RW.

__Since SceVfsMountPoC is a PoC, there is nothing that guarantees its operation.__

TODO : update createFileEntry and vfs_get_stat

# KernelCustomMalloc

Malloc with custom memory base

Basically the same as sceKernelAllocHeapMemory

# CorelockSample

The Corelock Sample.

# act_verifier

Verify DevKit/TestingKit activation file.

# kpanic_debug

Switch dbgFingerprint display to module name display.

note

To view the kpanic log, you need Princess Log or an equivalent logger.

Currently there is no more functionality.

# syscon_vs_dumper

Dumper for syscon vs storage.

vs storage includes resume context address, rtc tick, dipsw, etc.
