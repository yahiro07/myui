set(NAME briq-core)
add_library(${NAME} STATIC)

target_sources(${NAME} PRIVATE dummy-entry.cpp)
