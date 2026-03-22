set(NAME dev0)

add_executable(${NAME} main.cpp dev0.cpp)

target_link_libraries(${NAME} PRIVATE briq-lib)
