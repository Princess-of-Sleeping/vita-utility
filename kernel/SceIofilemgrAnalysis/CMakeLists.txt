cmake_minimum_required(VERSION 3.0)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(SceIofilemgrAnalysis)
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-q -Wall -O3 -fno-inline")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions")

enable_language(C ASM)

add_executable(SceIofilemgrAnalysis
  src/main.c
  src/log.c
)

target_link_libraries(SceIofilemgrAnalysis
  SceIofilemgrForDriver_stub
  SceSysclibForDriver_stub
  SceSysrootForDriver_stub
  SceProcessmgrForDriver_stub
  SceThreadmgrForDriver_stub
  SceModulemgrForDriver_stub
  SceModulemgrForKernel_stub
  SceSysmemForDriver_stub
)

set_target_properties(SceIofilemgrAnalysis
  PROPERTIES LINK_FLAGS "-nostdlib"
)

vita_create_self(SceIofilemgrAnalysis.skprx SceIofilemgrAnalysis
  CONFIG exports.yml
  UNSAFE
)

vita_create_stubs(stubs_SceIofilemgrAnalysis SceIofilemgrAnalysis ${CMAKE_CURRENT_SOURCE_DIR}/exports.yml KERNEL)
