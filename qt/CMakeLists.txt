cmake_minimum_required(VERSION 3.10)

project(keyboxd_ui)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
# Create code from a list of Qt designer ui files
set(CMAKE_AUTOUIC ON)
set(CMAKE_MACOSX_BUNDLE TRUE)

# Populate a CMake variable with the sources
set(keyboxd_ui_SRCS
    mainwindow.ui
    mainwindow.cpp
    main.cpp
)
find_package(Qt5Widgets CONFIG REQUIRED)

add_executable(keyboxd_ui  ${keyboxd_ui_SRCS})
target_link_libraries(keyboxd_ui Qt5::Widgets)

