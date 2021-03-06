cmake_minimum_required(VERSION 3.0)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(mapping_vaddr_sample)
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-q -Wall -O3 -fno-inline")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fno-rtti -fno-exceptions")

enable_language(C ASM)

link_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
)

add_executable(mapping_vaddr_sample
  src/main.c
)

target_link_libraries(mapping_vaddr_sample
  SceIofilemgrForDriver_stub
  SceSysclibForDriver_stub
  SceDebugForDriver_stub
  SceCpuForKernel_stub
  taihenForKernel_stub
  taihenModuleUtils_stub
)

set_target_properties(mapping_vaddr_sample
  PROPERTIES LINK_FLAGS "-nostdlib"
)

vita_create_self(mapping_vaddr_sample.skprx mapping_vaddr_sample
  CONFIG exports.yml
  UNSAFE
)
