cmake_minimum_required(VERSION 2.8)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(map_user_memblk_by_paddr)
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-q -Wall -O3 -fno-inline")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions")

add_executable(map_user_memblk_by_paddr
  src/main.c
)

target_link_libraries(map_user_memblk_by_paddr
  SceDebugForDriver_stub
  SceProcEventForDriver_stub
  SceSysclibForDriver_stub
  SceSysmemForDriver_stub
)

set_target_properties(map_user_memblk_by_paddr
  PROPERTIES LINK_FLAGS "-nostdlib"
)

vita_create_self(map_user_memblk_by_paddr.skprx map_user_memblk_by_paddr
  CONFIG exports.yml
  UNSAFE
)