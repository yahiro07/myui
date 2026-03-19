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

add_executable(${PROJECT_NAME} main.cpp dev0.cpp cocoa-window-floor.mm)
set_source_files_properties(cocoa-window-floor.mm PROPERTIES COMPILE_OPTIONS
                                                             "-fobjc-arc")
target_link_libraries(${PROJECT_NAME} blend2d::blend2d "-framework Cocoa")
