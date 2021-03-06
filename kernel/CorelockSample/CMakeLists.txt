cmake_minimum_required(VERSION 3.0)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(corelock_sample)
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-q -Wall -O3 -fno-inline")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions")

enable_language(C ASM)

add_executable(corelock_sample
  src/main.c
)

target_link_libraries(corelock_sample
  SceCpuForDriver_stub
  SceCpuForKernel_stub
  SceDebugForDriver_stub
  SceThreadmgrForDriver_stub
  taihenModuleUtils_stub
)

set_target_properties(corelock_sample
  PROPERTIES LINK_FLAGS "-nostdlib"
)

vita_create_self(corelock_sample.skprx corelock_sample
  CONFIG exports.yml
  UNSAFE
)
