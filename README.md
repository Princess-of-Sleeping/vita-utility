# vita-utility

### MappingVaddrSample

for kernel

Sample of how to map vaddr from paddr

### PafCustomMalloc

for userland

Malloc with custom memory base

### ScePfsMgrAllowDbgPfsPatch

Patch to allow debug version sce_pfs app mount and pkg install. 

### SceVfsMountPoC

for kernel

Create host0 as a virtual device.

The files that exist in "host0:kernel_modules/" and "host0:sysroot/" are RW.

__Since SceVfsMountPoC is a PoC, there is nothing that guarantees its operation.__

TODO : update createFileEntry and vfs_get_stat

### KernelCustomMalloc

for kernel

Malloc with custom memory base

Basically the same as sceKernelAllocHeapMemory

### CpuCoreSync

for kernel

Synchronize all cores and wait for synchronization to occur

### act_verifier

Verify DevKit/TestingKit activation file.

### kpanic_debug

Switch dbgFingerprint display to module name display.

note

To view the kpanic log, you need Princess Log or an equivalent logger.

Currently there is no more functionality.

# Special thanks:

HENkaku wiki
