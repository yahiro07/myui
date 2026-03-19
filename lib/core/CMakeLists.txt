set(NAME myui-core)
add_library(${NAME} STATIC)

target_sources(${NAME} PRIVATE ui-actor.cpp node-handle.cpp renderer.cpp)

target_link_libraries(${NAME} PUBLIC blend2d::blend2d)
