set(NAME myui-editor-frame)
add_library(${NAME} STATIC)

target_sources(${NAME} PRIVATE editor-frame.mm)

set_source_files_properties(editor-frame.mm PROPERTIES COMPILE_OPTIONS
                                                       "-fobjc-arc")
