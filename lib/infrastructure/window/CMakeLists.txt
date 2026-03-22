set(NAME myui-window)
add_library(${NAME} STATIC)

target_sources(${NAME} PRIVATE cocoa-window.mm)

target_link_libraries(${NAME} PUBLIC "-framework Cocoa")

set_source_files_properties(cocoa-window.mm PROPERTIES COMPILE_OPTIONS
                                                       "-fobjc-arc")
