set(NAME myui-core)
add_library(${NAME} STATIC)

target_sources(${NAME} PRIVATE dummy-entry.cpp)
