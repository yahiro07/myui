set(NAME my-plugin)

add_executable(${NAME} main.cpp)

target_link_libraries(${NAME} PRIVATE briq-lib)
