cmake_minimum_required(VERSION 2.8)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(SceUsbstorVStorFat16)
set(SELF_NAME "usbstor_fat16")
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-q -Wall -O3 -fno-inline -fno-builtin")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions")

add_executable(${PROJECT_NAME}
  src/main.c
)

target_link_libraries(${PROJECT_NAME}
  SceSysmemForDriver_stub
  SceSysclibForDriver_stub
  SceModulemgrForDriver_stub
  SceThreadmgrForDriver_stub
  SceIofilemgrForDriver_stub
  taihenForKernel_stub
  taihenModuleUtils_stub
)

set_target_properties(${PROJECT_NAME}
  PROPERTIES LINK_FLAGS "-nostdlib"
  COMPILE_FLAGS "-D__VITA_KERNEL__"
)

vita_create_self(${SELF_NAME}.skprx ${PROJECT_NAME}
  CONFIG exports.yml
  UNSAFE
)

vita_create_stubs(stubs_${PROJECT_NAME} ${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/exports.yml KERNEL)
