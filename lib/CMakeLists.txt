set(NAME myui-lib)

add_subdirectory(core)
add_subdirectory(drawings)
add_subdirectory(editor-frame)
add_subdirectory(window)

add_library(${NAME} STATIC)

target_sources(${NAME} PRIVATE myui-lib.cpp)

target_link_libraries(${NAME} PUBLIC myui-core myui-drawings myui-editor-frame
                                     myui-window)

target_include_directories(${NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
