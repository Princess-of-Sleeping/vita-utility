cmake_minimum_required(VERSION 2.8)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(SceExfatfsAnalysis)
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-q -Wall -O3 -fno-inline")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions")

link_directories(
  ${CMAKE_SOURCE_DIR}/
)

enable_language(C ASM)

add_executable(SceExfatfsAnalysis
  src/main.c
  src/log.c
)

target_link_libraries(SceExfatfsAnalysis
  SceDebugForDriver_stub
  SceIofilemgrForDriver_stub
  SceSysclibForDriver_stub
  SceModulemgrForDriver_stub
  SceModulemgrForKernel_stub
  SceSysmemForDriver_stub
  taihenForKernel_stub
)

set_target_properties(SceExfatfsAnalysis
  PROPERTIES LINK_FLAGS "-nostdlib"
)

vita_create_self(SceExfatfsAnalysis.skprx SceExfatfsAnalysis
  CONFIG exports.yml
  UNSAFE
)

vita_create_stubs(stubs SceExfatfsAnalysis ${CMAKE_SOURCE_DIR}/exports.yml KERNEL)
