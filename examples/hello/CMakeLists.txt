set(NAME hello)

add_executable(${NAME} main.cpp)

target_link_libraries(${NAME} PRIVATE myui-lib)
