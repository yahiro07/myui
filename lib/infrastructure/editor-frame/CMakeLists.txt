set(NAME briq-editor-frame)
add_library(${NAME} STATIC)

target_sources(${NAME} PRIVATE editor-frame.mm)

target_link_libraries(${NAME} PUBLIC "-framework Cocoa")

set_source_files_properties(editor-frame.mm PROPERTIES COMPILE_OPTIONS
                                                       "-fobjc-arc")
