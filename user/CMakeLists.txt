cmake_minimum_required(VERSION 3.0)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(vita-utility-user)

add_subdirectory(PafCustomMalloc)
add_subdirectory(SceSysRw)
add_subdirectory(kernel_module_loader)
add_subdirectory(malloc_free_test_user)
add_subdirectory(repaf)
add_subdirectory(npdrm_act_guard)
add_subdirectory(VideoDebugger)
add_subdirectory(npdrm_package)
