set(NAME myui-window)
add_library(${NAME} STATIC)

target_sources(${NAME} PRIVATE cocoa-window-floor.mm)

target_link_libraries(${NAME} PUBLIC blend2d::blend2d "-framework Cocoa")

set_source_files_properties(cocoa-window-floor.mm PROPERTIES COMPILE_OPTIONS
                                                             "-fobjc-arc")
