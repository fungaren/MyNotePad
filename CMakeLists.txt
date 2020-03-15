cmake_minimum_required(VERSION 2.8)
project(mynotepad)

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
    # if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
    #     set(default_build_type "Debug")
    # endif()

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
set(MYNOTEPAD_VERSION_MINOR 0)
set(MYNOTEPAD_VERSION_PATCH 5)
message(STATUS "MyNotePad version: ${MYNOTEPAD_VERSION_MAJOR}."
        "${MYNOTEPAD_VERSION_MINOR}.${MYNOTEPAD_VERSION_PATCH}")

set(MYNOTEPAD_INSTALL_PATH "${CMAKE_INSTALL_PREFIX}/share")
configure_file(
    "${PROJECT_SOURCE_DIR}/inc/config.h.in"
    "${PROJECT_BINARY_DIR}/inc/config.h"
)

if (UNIX)
    configure_file(
        "${PROJECT_SOURCE_DIR}/static/mynotepad.desktop.in"
        "${PROJECT_SOURCE_DIR}/static/mynotepad.desktop"
    )
endif()

# specify the path manually
if (WIN32)
    #set(wxWidgets_ROOT_DIR "D:/wxWidgets-3.1.3/")
    #set(wxWidgets_LIB_DIR "D:/wxWidgets-3.1.3/lib/vc14x_dll")
endif()
set(wxWidgets_CONFIGURATION mswu) # or mswud

# find wxWidgets
find_package(wxWidgets COMPONENTS core base stc REQUIRED)
message(STATUS "wxWidgets version: ${wxWidgets_VERSION_MAJOR}."
        "${wxWidgets_VERSION_MINOR}.${wxWidgets_VERSION_PATCH}")

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
    set(WXVER "${wxWidgets_VERSION_MAJOR}${wxWidgets_VERSION_MINOR}${wxWidgets_VERSION_PATCH}")
    # required DLLs on Windows
    set(ALL_DLLS_DEBUG
        "${wxWidgets_LIB_DIR}/wxbase${WXVER}ud_vc14x_x64.dll"
        "${wxWidgets_LIB_DIR}/wxbase${WXVER}ud_net_vc14x_x64.dll"
        "${wxWidgets_LIB_DIR}/wxbase${WXVER}ud_xml_vc14x_x64.dll"
        "${wxWidgets_LIB_DIR}/wxmsw${WXVER}ud_core_vc14x_x64.dll"
        "${wxWidgets_LIB_DIR}/wxmsw${WXVER}ud_stc_vc14x_x64.dll"
        )
    set(ALL_DLLS_RELEASE
        "${wxWidgets_LIB_DIR}/wxbase${WXVER}u_vc14x_x64.dll"
        "${wxWidgets_LIB_DIR}/wxbase${WXVER}u_net_vc14x_x64.dll"
        "${wxWidgets_LIB_DIR}/wxbase${WXVER}u_xml_vc14x_x64.dll"
        "${wxWidgets_LIB_DIR}/wxmsw${WXVER}u_core_vc14x_x64.dll"
        "${wxWidgets_LIB_DIR}/wxmsw${WXVER}u_stc_vc14x_x64.dll"
        )
    set(ALL_DLLS $<$<CONFIG:Debug>:${ALL_DLLS_DEBUG}>$<$<CONFIG:Release>:${ALL_DLLS_RELEASE}>)
    #message(STATUS ${ALL_DLLS})
    set(STATIC_RESOURCES
        "${PROJECT_SOURCE_DIR}/static/highlight.css"
        "${PROJECT_SOURCE_DIR}/static/highlight.pack.js"
        "${PROJECT_SOURCE_DIR}/static/style.css"
        "${PROJECT_SOURCE_DIR}/static/logo.png"
        "${PROJECT_SOURCE_DIR}/static/mynotepad.visualelementsmanifest.xml"
        )
    # destination directory
    set(COPY_DLLS_TO ${PROJECT_BINARY_DIR}/)
    #message(STATUS ${COPY_DLLS_TO})
    add_custom_command(TARGET ${TARGT} POST_BUILD
        COMMAND_EXPAND_LISTS
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${ALL_DLLS}" "${COPY_DLLS_TO}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${STATIC_RESOURCES}" "${COPY_DLLS_TO}"
        COMMENT "Copying DLLs" VERBATIM
        )
endif()

if (UNIX)
    # default path is /usr/local/
    message(STATUS "CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")

    # install the program after build, /usr/local/bin/
    install(TARGETS ${TARGT} RUNTIME DESTINATION bin)
    
    # copy dependent files while installing, default path is /usr/local/share
    install(DIRECTORY "${PROJECT_SOURCE_DIR}/static/"
            DESTINATION "share/${PROJECT_NAME}"
            # make the files writable
            FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ WORLD_WRITE
            FILES_MATCHING PATTERN "*.css" PATTERN "*.js" PATTERN "*.png"
            )
    
    # create desktop icon
    install(FILES "${PROJECT_SOURCE_DIR}/static/mynotepad.desktop"
            DESTINATION /usr/share/applications
            PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
            )
endif()
