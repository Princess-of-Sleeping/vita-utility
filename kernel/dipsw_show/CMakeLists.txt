cmake_minimum_required(VERSION 2.8)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(dipsw_show)
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-q -Wall -O3 -fno-inline")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions")

enable_language(C ASM)

add_executable(dipsw_show
  src/main.c
)

target_link_libraries(dipsw_show
  SceDipswForDriver_stub
  SceDebugForDriver_stub
)

set_target_properties(dipsw_show
  PROPERTIES LINK_FLAGS "-nostdlib"
)

vita_create_self(dipsw_show.skprx dipsw_show
  CONFIG exports.yml
  UNSAFE
)

vita_create_stubs(stubs_dipsw_show dipsw_show ${CMAKE_CURRENT_SOURCE_DIR}/exports.yml KERNEL)
