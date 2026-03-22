cmake_minimum_required(VERSION 3.30)

project(prototype_app CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)

# ----------
# setup blend2d

include(FetchContent)

set(BLEND2D_STATIC TRUE)
set(BLEND2D_NO_JIT TRUE)

FetchContent_Declare(
  blend2d
  GIT_REPOSITORY https://github.com/blend2d/blend2d.git
  GIT_TAG master
  GIT_SHALLOW TRUE
  UPDATES_DISCONNECTED ON)
FetchContent_MakeAvailable(blend2d)

# ----------

add_subdirectory(lib)

option(BRIQ_BUILD_EXAMPLES "Build examples" OFF)

if(BRIQ_BUILD_EXAMPLES)
  add_subdirectory(examples/hello)
  add_subdirectory(examples/dev0)
endif()
