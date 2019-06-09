cmake_minimum_required(VERSION 2.8)
project(MyNotePad)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")

set(MYNOTEPAD_VERSION_MAJOR 1)
set(MYNOTEPAD_VERSION_MINOR 0.5)

configure_file(
    "${PROJECT_SOURCE_DIR}/inc/config.h.in"
    "${PROJECT_BINARY_DIR}/inc/config.h"
)

# set(wxWidgets_ROOT_DIR <wxWidgets Directory>)
# set(wxWidgets_CONFIGURATION mswud)
set(wxWidgets_CONFIGURATION mswu)
find_package(wxWidgets COMPONENTS core base REQUIRED)
include(${wxWidgets_USE_FILE})
set(EXTRA_LIBS ${EXTRA_LIBS} ${wxWidgets_LIBRARIES})

option(USE_NATIVE_EDIT_BOX "Use native or custom implemented edit control" ON)
add_subdirectory(md2html)
set(EXTRA_LIBS ${EXTRA_LIBS} md2html)

include_directories(
    "${PROJECT_BINARY_DIR}/inc"
    "${PROJECT_SOURCE_DIR}/inc"
    "${PROJECT_SOURCE_DIR}/static"
    "${PROJECT_SOURCE_DIR}/md2html/inc"
)
aux_source_directory(src SOURCES)

if (WIN32)
    list(APPEND SOURCES "static/wx.rc")
endif()

set(EXECUTABLE_NAME mynotepad)
add_executable(${EXECUTABLE_NAME} ${SOURCES})
target_link_libraries(${EXECUTABLE_NAME} ${EXTRA_LIBS})

install(TARGETS ${EXECUTABLE_NAME} RUNTIME DESTINATION bin)
install(DIRECTORY "${PROJECT_SOURCE_DIR}/static" DESTINATION "share/${PROJECT_NAME}")
