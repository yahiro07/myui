set(NAME myui-core)
add_library(${NAME} STATIC)

target_sources(${NAME} PRIVATE drawings/renderer.cpp)

target_link_libraries(${NAME} PUBLIC blend2d::blend2d)
