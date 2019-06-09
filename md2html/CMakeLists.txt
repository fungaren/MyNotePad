
include_directories(inc)
aux_source_directory(src SOURCES)
add_library(md2html STATIC ${SOURCES})
