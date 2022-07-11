cmake_minimum_required(VERSION 3.0)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(netps_kernel_recv_fix)
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-q -Wall -O2 -fno-inline")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions")

add_executable(netps_kernel_recv_fix
  src/main.c
)

target_link_libraries(netps_kernel_recv_fix
  SceModulemgrForDriver_stub
  SceSysclibForDriver_stub
  SceKernelUtilsForDriver_stub
  taihenForKernel_stub
  taihenModuleUtils_stub
)

set_target_properties(netps_kernel_recv_fix
  PROPERTIES LINK_FLAGS "-nostdlib"
  COMPILE_FLAGS "-D__PSP2_KERNEL__"
)

vita_create_self(netps_kernel_recv_fix.skprx netps_kernel_recv_fix
  CONFIG exports.yml
  UNSAFE
  STRIPPED
  REL_OPTIMIZE
)

vita_create_stubs(stubs_${PROJECT_NAME} netps_kernel_recv_fix exports.yml KERNEL)
