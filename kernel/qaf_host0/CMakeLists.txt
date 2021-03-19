cmake_minimum_required(VERSION 3.0)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(SceKernelQafHost0)
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-q -Wall -O3 -fno-inline")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions")

add_executable(SceKernelQafHost0
  src/main.c
)

target_link_libraries(SceKernelQafHost0
  SceSysrootForKernel_stub
)

set_target_properties(SceKernelQafHost0
  PROPERTIES LINK_FLAGS "-nostdlib"
)

vita_create_self(qaf_host0.skprx SceKernelQafHost0
  CONFIG exports.yml
  UNSAFE
)