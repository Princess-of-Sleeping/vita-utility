cmake_minimum_required(VERSION 2.8)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(SceVfsMountPoC)
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-q -Wall -O3 -fno-inline")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions")

enable_language(C ASM)

add_executable(SceVfsMountPoC
  src/main.c
  src/cpu.S
  src/sce_cpu.c
  src/item_mgr.c
  src/fs_mgr.c
  src/module_kernel.c
  src/sysroot_kblparam.c
  src/etc.c
)

target_link_libraries(SceVfsMountPoC
  SceCpuForDriver_stub
  SceDebugForDriver_stub
  SceIofilemgrForDriver_stub
  SceSysclibForDriver_stub
  SceSblSsMgrForDriver_stub
  SceSysmemForDriver_stub
  SceModulemgrForDriver_stub
  SceSysrootForKernel_stub
  taihenModuleUtils_stub
)

set_target_properties(SceVfsMountPoC
  PROPERTIES LINK_FLAGS "-nostdlib"
)

vita_create_self(SceVfsMountPoC.skprx SceVfsMountPoC
  CONFIG exports.yml
  UNSAFE
)
