cmake_minimum_required(VERSION 3.0)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(idstorage_dumper)
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-q -Wall -O3 -fno-inline")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions")

add_executable(idstorage_dumper
  src/main.c
)

target_link_libraries(idstorage_dumper
  SceDebugForDriver_stub
  SceSysmemForDriver_stub
  SceSysclibForDriver_stub
  SceIdStorageForDriver_stub
  SceIofilemgrForDriver_stub
  SceThreadmgrForDriver_stub
)

set_target_properties(idstorage_dumper
  PROPERTIES LINK_FLAGS "-nostdlib"
)

vita_create_self(idstorage_dumper.skprx idstorage_dumper
  CONFIG exports.yml
  UNSAFE
)