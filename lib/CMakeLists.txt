set(NAME myui-lib)

add_subdirectory(core)
add_subdirectory(drawings)
add_subdirectory(infrastructure/editor-frame)
add_subdirectory(infrastructure/window)

add_library(${NAME} STATIC)

target_sources(${NAME} PRIVATE dummy-entry.cpp)

target_link_libraries(${NAME} PUBLIC myui-core myui-drawings myui-editor-frame
                                     myui-window)

target_include_directories(${NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
