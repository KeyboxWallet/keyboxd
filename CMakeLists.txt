
cmake_minimum_required (VERSION 3.1)
set (CMAKE_CXX_STANDARD 11)
project (keybox_daemon)

if (STATIC_BUILD)
SET( Boost_USE_STATIC_LIBS ON  )
SET( Protobuf_USE_STATIC_LIBS ON )
endif (STATIC_BUILD)

FIND_PACKAGE( Boost 1.67 COMPONENTS system REQUIRED )
FIND_PACKAGE (Protobuf REQUIRED)

INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )
if (UNIX)
 set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")
endif()
INCLUDE_DIRECTORIES( ../json_rpc_lib ../json_lib/include )
if (UNIX)
 find_package(PkgConfig)
 pkg_check_modules (LIBUSB REQUIRED libusb-1.0)
elseif (WIN32)
 if (NOT IS_DIRECTORY $ENV{vcpkg_root})
  message(FATAL_ERROR "you must specify vcpkg_root for seeking packages")
 endif()
 message(STATUS "win32 selected. seeking libusb")
 if (NOT IS_DIRECTORY $ENV{vcpkg_root}/installed/x86-windows/include/libusb-1.0)
  message(FATAL_ERROR "please install libusb var vcpkg")
 endif()
 set(LIBUSB_CFLAGS -D_WIN32_WINNT=0x0601 -I$ENV{vcpkg_root}/installed/x86-windows/include/libusb-1.0)
 set(LIBUSB_LIBRARIES_DIR_DEBUG $ENV{vcpkg_root}/installed/x86-windows/debug)
 set(LIBUSB_LIBRARIES_DIR_REL $ENV{vcpkg_root}/installed/x86-windows)
 SET(LIBUSB_LIBRARIES_DEBUG ${LIBUSB_LIBRARIES_DIR_DEBUG}/lib/libusb-1.0.lib)
 SET(LIBUSB_LIBRARIES_REL ${LIBUSB_LIBRARIES_DIR_REL}/lib/libusb-1.0.lib)
endif()
INCLUDE_DIRECTORIES(../protodef)
INCLUDE_DIRECTORIES(../protodef/cpp)

add_executable( keyboxd main.cpp
                rpc-server.cpp
                device-manager.cpp
                base-device.cpp
                local-device.cpp
                usb-device.cpp
                base64.cpp
                ../protodef/cpp/messages.pb.cc )
TARGET_LINK_LIBRARIES(keyboxd ${Boost_LIBRARIES} ${Protobuf_LIBRARIES})

target_compile_options (keyboxd PUBLIC ${LIBUSB_CFLAGS} )
target_link_libraries (keyboxd debug ${LIBUSB_LIBRARIES} ${LIBUSB_LIBRARIES_DEBUG})
TARGET_LINK_LIBRARIES(keyboxd optimized  ${LIBUSB_LIBRARIES} ${LIBUSB_LIBRARIES_REL})