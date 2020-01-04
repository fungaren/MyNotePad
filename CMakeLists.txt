cmake_minimum_required(VERSION 2.8)
project(MyNotePad)

if (COMMAND cmake_policy)
    # Works around warnings libraries linked against that don't
    # have absolute paths (e.g. -lpthreads)
    cmake_policy(SET CMP0003 NEW)

    # Qt5 qt5_use_modules usage was causing "Policy CMP0043 is not set: 
    # Ignore COMPILE_DEFINITIONS_<Config> properties." warnings
    cmake_policy(SET CMP0043 NEW)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
#add_definitions("$<$<CONFIG:Debug>:-D_DEBUG>")

if (UNIX)
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)

    # Set a default build type if none was specified
    set(default_build_type "Release")
    if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
        set(default_build_type "Debug")
    endif()
     
    if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
        message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
        set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
            STRING "Choose the type of build." FORCE)
        # Set the possible values of build type for cmake-gui
        set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
            "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
    endif()
endif()

set(MYNOTEPAD_VERSION_MAJOR 1)
set(MYNOTEPAD_VERSION_MINOR 0.5)

#option(USE_NATIVE_EDIT_BOX "Use native or custom implemented edit control" ON)

configure_file(
    "${PROJECT_SOURCE_DIR}/inc/config.h.in"
    "${PROJECT_BINARY_DIR}/inc/config.h"
)

# for Windows we have to specify the path manually
if (WIN32)
    set(wxWidgets_ROOT_DIR "D:/wxWidgets-3.1.2/")
    set(wxWidgets_LIB_DIR "D:/wxWidgets-3.1.2/lib/vc141_dll")
endif()
set(wxWidgets_CONFIGURATION mswu) # or mswud

# find wxWidgets
find_package(wxWidgets COMPONENTS core base stc REQUIRED)

include(${wxWidgets_USE_FILE})
set(EXTRA_LIBS ${EXTRA_LIBS} ${wxWidgets_LIBRARIES})

# submodule md2html
add_subdirectory(md2html)
set(EXTRA_LIBS ${EXTRA_LIBS} md2html)

# define header directory
include_directories(
    "${PROJECT_BINARY_DIR}/inc"
    "${PROJECT_SOURCE_DIR}/inc"
    "${PROJECT_SOURCE_DIR}/static"
    "${PROJECT_SOURCE_DIR}/md2html/inc"
)
# define source directory
aux_source_directory(src SOURCES)

if (WIN32)
    # add resource file for Windows
    list(APPEND SOURCES "static/wx.rc")
endif()

# target executable file
set(TARGT mynotepad)
add_executable(${TARGT} WIN32 ${SOURCES})

# link libraries
target_link_libraries(${TARGT} ${EXTRA_LIBS})

if (WIN32)
    # required DLLs on Windows
    set(ALL_DLLS_DEBUG
        "${wxWidgets_LIB_DIR}/wxbase312ud_vc141.dll"
        "${wxWidgets_LIB_DIR}/wxbase312ud_net_vc141.dll"
        "${wxWidgets_LIB_DIR}/wxbase312ud_xml_vc141.dll"
        "${wxWidgets_LIB_DIR}/wxmsw312ud_core_vc141.dll"
        "${wxWidgets_LIB_DIR}/wxmsw312ud_stc_vc141.dll"
        )
    set(ALL_DLLS_RELEASE
        "${wxWidgets_LIB_DIR}/wxbase312u_vc141.dll"
        "${wxWidgets_LIB_DIR}/wxbase312u_net_vc141.dll"
        "${wxWidgets_LIB_DIR}/wxbase312u_xml_vc141.dll"
        "${wxWidgets_LIB_DIR}/wxmsw312u_core_vc141.dll"
        "${wxWidgets_LIB_DIR}/wxmsw312u_stc_vc141.dll"
        )
    set(ALL_DLLS $<$<CONFIG:Debug>:${ALL_DLLS_DEBUG}>$<$<CONFIG:Release>:${ALL_DLLS_RELEASE}>)
    #message(${ALL_DLLS})
    set(STATIC_RESOURCES
        "${PROJECT_SOURCE_DIR}/static/highlight.css"
        "${PROJECT_SOURCE_DIR}/static/highlight.pack.js"
        "${PROJECT_SOURCE_DIR}/static/style.css"
        "${PROJECT_SOURCE_DIR}/static/logo.png"
        "${PROJECT_SOURCE_DIR}/static/mynotepad.visualelementsmanifest.xml"
        )
    # destination directory
    set(COPY_DLLS_TO ${PROJECT_BINARY_DIR}/$<$<CONFIG:Release>:Release>$<$<CONFIG:Debug>:Debug>/)
    #message(${COPY_DLLS_TO})
    add_custom_command(TARGET ${TARGT} POST_BUILD
        COMMAND_EXPAND_LISTS
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${ALL_DLLS}" "${COPY_DLLS_TO}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${STATIC_RESOURCES}" "${COPY_DLLS_TO}"
        COMMENT "Copying DLLs" VERBATIM
        )
endif()

if (UNIX)
    # install the program after build, default path is /usr/local/bin/
    install(TARGETS ${TARGT} RUNTIME DESTINATION bin)
    # copy dependent files while installing, default path is /usr/local/share
    install(DIRECTORY "${PROJECT_SOURCE_DIR}/static" 
            DESTINATION "share/${PROJECT_NAME}" 
            FILES_MATCHING PATTERN "*.css" PATTERN "*.js"
            )
endif()
