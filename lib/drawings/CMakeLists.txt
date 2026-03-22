set(NAME myui-drawings)
add_library(${NAME} STATIC)

target_sources(${NAME} PRIVATE renderer.cpp)

target_link_libraries(${NAME} PUBLIC blend2d::blend2d)
