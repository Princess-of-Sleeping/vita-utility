cmake_minimum_required(VERSION 3.0)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(act_verifier)
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-q -Wall -O3 -fno-inline")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions")

add_executable(act_verifier
  src/main.c
  src/aes256cmac.c
)

target_link_libraries(act_verifier
  SceDebugForDriver_stub
  SceIofilemgrForDriver_stub
  SceModulemgrForDriver_stub
  SceThreadmgrForDriver_stub
  SceSblSsMgrForDriver_stub
  SceSblSsMgrForKernel_stub
  SceSysclibForDriver_stub
  SceKernelUtilsForDriver_stub
  taihenModuleUtils_stub
)

set_target_properties(act_verifier
  PROPERTIES LINK_FLAGS "-nostdlib"
)

vita_create_self(act_verifier.skprx act_verifier
  CONFIG exports.yml
  UNSAFE
)
