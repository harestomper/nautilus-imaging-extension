cmake_minimum_required (VERSION 2.6) 
 
set (PROJECT nautilus-imaging-extension)
set (CMAKE_INSTALL_PREFIX "/usr")

project (${PROJECT}) 

find_package(PkgConfig REQUIRED)

#
# GTK & Nautilus
#

pkg_check_modules (GTK REQUIRED gtk+-3.0)
pkg_check_modules (GLIB REQUIRED glib-2.0)
pkg_check_modules (NAUTILUS REQUIRED libnautilus-extension)
pkg_check_modules (MAGICK REQUIRED MagickWand)

#
# config.h
##
set (GETTEXT_PACKAGE ${PROJECT})
set (GNOMELOCALEDIR "/usr/share/locale")
set (PACKAGE_NAME ${PROJECT})
set (PACKAGE_VERSION "0.0.1")
set (PACKAGE_STRING "${PROJECT}-${PACKAGE_VERSION}")
configure_file (${CMAKE_SOURCE_DIR}/config.h.cmake ${CMAKE_BINARY_DIR}/config.h)

#
# Set include dirs
##
include_directories(
                    ${GTK_INCLUDE_DIRS}
                    ${GTK_LIBRARIES}
                    ${GLIB_INCLUDE_DIRS}
                    ${GLIB_LIBRARIES}
                    ${CMAKE_CURRENT_BINARY_DIR}/src
                    ${CMAKE_SOURCE_DIR}
                    ${MAGICK_INCLUDE_DIRS}
                    ${MAGICK_LIBRARIES}
                    ${NAUTILUS_INCLUDE_DIRS}
                    ${NAUTILUS_LIBRARIES}
                   )

    

#
# i18n
#

find_package (Gettext REQUIRED)

file (GLOB _translations ${CMAKE_SOURCE_DIR}/po/*.po)
        GETTEXT_CREATE_TRANSLATIONS (${CMAKE_SOURCE_DIR}/po/${PROJECT_NAME}.pot
        ALL ${_translations})

#
# Set sources for compiling
##
FILE (GLOB SOURCES ${CMAKE_SOURCE_DIR}/src/nimdialog.c
                   ${CMAKE_SOURCE_DIR}/src/nimimaging.c
                   ${CMAKE_SOURCE_DIR}/src/main.c)

message (STATUS ${SOURCES})
#
# Set config.h
##

set (CMAKE_C_FLAGS "-include config.h -lm")

#
# Build shared library
##

# add_library (${PROJECT}  SHARED ${SOURCES})
add_executable (${PROJECT} ${SOURCES})

target_link_libraries(${PROJECT}
                    ${GTK_LIBRARIES}
                    ${GLIB_LIBRARIES}
                    ${MAGICK_LIBRARIES}
                    ${NAUTILUS_LIBRARIES})

#
# Install files
#

set (CMAKE_INSTALL_PREFIX "/usr")

install(TARGETS ${PROJECT} DESTINATION bin/)
# install(FILES pixmaps/itmages-screenshot.png DESTINATION share/pixmaps/)
# install(FILES data/itmages-screenshot.desktop DESTINATION share/applications/)
#install(FILES data/org.gnome.itmages.gschema.xml DESTINATION share/glib-2.0/schemas/)
