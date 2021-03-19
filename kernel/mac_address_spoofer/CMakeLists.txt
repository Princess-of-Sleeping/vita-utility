cmake_minimum_required(VERSION 3.0)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(mac_address_spoofer)
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-q -Wall -O3 -fno-inline")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions")

enable_language(C ASM)

add_executable(mac_address_spoofer
  src/main.c
)

target_link_libraries(mac_address_spoofer
  SceSysclibForDriver_stub
  SceModulemgrForDriver_stub
  SceThreadmgrForDriver_stub
  taihenForKernel_stub
  taihenModuleUtils_stub
)

set_target_properties(mac_address_spoofer
  PROPERTIES LINK_FLAGS "-nostdlib"
)

vita_create_self(mac_address_spoofer.skprx mac_address_spoofer
  CONFIG exports.yml
  UNSAFE
)

vita_create_stubs(stubs_mac_address_spoofer mac_address_spoofer ${CMAKE_CURRENT_SOURCE_DIR}/exports.yml KERNEL)
