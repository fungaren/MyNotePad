
include_directories(inc)
aux_source_directory(src MD_SOURCES)
add_library(md2html STATIC ${MD_SOURCES})
