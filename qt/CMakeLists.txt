cmake_minimum_required(VERSION 3.10)

project(keyboxd_ui)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
# Create code from a list of Qt designer ui files
set(CMAKE_AUTOUIC ON)
set(CMAKE_MACOSX_BUNDLE TRUE)
SET(MACOSX_BUNDLE_INFO_STRING "keybox daemon application")
SET(MACOSX_BUNDLE_ICON_FILE "icon")
SET(MACOSX_BUNDLE_GUI_IDENTIFIER keyboxd_ui)
SET(MACOSX_BUNDLE_LONG_VERSION_STRING "initial version")
SET(MACOSX_BUNDLE_BUNDLE_NAME "keyboxd_ui")
SET(MACOSX_BUNDLE_SHORT_VERSION_STRING "0.1.0")
SET(MACOSX_BUNDLE_BUNDLE_VERSION "0.1.0")
SET(MACOSX_BUNDLE_COPYRIGHT "MIT, by keybox team")

set( ICON_NAME "icon.icns" )
set( ICON_PATH "${PROJECT_SOURCE_DIR}/${ICON_NAME}" )

SET_SOURCE_FILES_PROPERTIES(${ICON_PATH} PROPERTIES
                             MACOSX_PACKAGE_LOCATION Resources)

# Populate a CMake variable with the sources
set(keyboxd_ui_SRCS
    mainwindow.ui
    mainwindow.cpp
    main.cpp
)
find_package(Qt5Widgets CONFIG REQUIRED)

add_executable(keyboxd_ui MACOSX_BUNDLE ${keyboxd_ui_SRCS} ${ICON_NAME})
target_link_libraries(keyboxd_ui Qt5::Widgets)
